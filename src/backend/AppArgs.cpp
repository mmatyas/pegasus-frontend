// Pegasus Frontend
// Copyright (C) 2017-2018  Mátyás Mustoha
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.


#include "AppArgs.h"

#include "ConfigFile.h"
#include "LocaleUtils.h"
#include "Paths.h"
#include "ScriptRunner.h"
#include "Utils.h"
#include "utils/HashMap.h"

#include <QDebug>
#include <QFile>
#include <QTextStream>


namespace {
QString config_path()
{
    return paths::writableConfigDir() + QStringLiteral("/settings.txt");
}

enum class ConfigEntryType : unsigned char {
    FULLSCREEN,
    LOCALE,
    THEME,
    ENABLE_ES2,
    ENABLE_STEAM,
};
struct ConfigEntryMap {
    const HashMap<const ConfigEntryType, const QString, EnumHash> type_to_str {
        { ConfigEntryType::FULLSCREEN, QStringLiteral("general.fullscreen") },
        { ConfigEntryType::LOCALE, QStringLiteral("general.locale") },
        { ConfigEntryType::THEME, QStringLiteral("general.theme") },
        { ConfigEntryType::ENABLE_ES2, QStringLiteral("providers.enable-es2") },
        { ConfigEntryType::ENABLE_STEAM, QStringLiteral("providers.enable-steam") },
    };
    HashMap<QString, const ConfigEntryType> str_to_type;

    ConfigEntryMap() {
        for (const auto& entry : type_to_str)
            str_to_type.emplace(entry.second, entry.first);
    }
};
} // namespace


bool AppArgs::portable_mode = false;
bool AppArgs::silent = false;
bool AppArgs::fullscreen = true;
bool AppArgs::enable_provider_es2 = true;
bool AppArgs::enable_provider_steam = true;
const QString AppArgs::DEFAULT_LOCALE(QStringLiteral("en"));
const QString AppArgs::DEFAULT_THEME(QStringLiteral(":/themes/pegasus-grid/"));
QString AppArgs::locale(DEFAULT_LOCALE);
QString AppArgs::theme(DEFAULT_THEME);


void AppArgs::load_config()
{
    const auto config_path = ::config_path();
    const ConfigEntryMap config_entry_map;


    const auto on_error = [&](const int lineno, const QString msg){
        qWarning().noquote()
            << tr_log("`%1`, line %2: %3").arg(config_path, QString::number(lineno), msg);
    };

    const auto on_attribute = [&](const int lineno, const QString key, const QString val){
        const auto entry_type = config_entry_map.str_to_type.find(key);
        if (entry_type == config_entry_map.str_to_type.cend()) {
            on_error(lineno, tr_log("unrecognized option `%1`, ignored").arg(key));
            return;
        }
        switch (entry_type->second) {
            case ConfigEntryType::FULLSCREEN:
            case ConfigEntryType::ENABLE_ES2:
            case ConfigEntryType::ENABLE_STEAM:
                if (!::is_str_bool(val)) {
                    on_error(lineno, QStringLiteral("this option requires a boolean (true/false) value"));
                    return;
                }
                break;
            default:
                break;
        }
        switch (entry_type->second) {
            case ConfigEntryType::FULLSCREEN:
                fullscreen = ::str_to_bool(val, fullscreen);
                break;
            case ConfigEntryType::LOCALE:
                locale = val;
                break;
            case ConfigEntryType::THEME:
                theme = val;
                break;
            case ConfigEntryType::ENABLE_ES2:
                enable_provider_es2 = ::str_to_bool(val, enable_provider_es2);
                break;
            case ConfigEntryType::ENABLE_STEAM:
                enable_provider_steam = ::str_to_bool(val, enable_provider_steam);
                break;
        }
    };

    config::readFile(config_path, on_attribute, on_error);
    qInfo().noquote() << tr_log("Program settings loaded");
}

void AppArgs::save_config()
{
    const auto config_path = ::config_path();

    QFile config_file(config_path);
    if (!config_file.open(QFile::WriteOnly | QFile::Text)) {
        qWarning().noquote() << tr_log("Failed to save program settings to `%1`")
                                .arg(config_path);
        return;
    }


    if (locale.isEmpty())
        locale = DEFAULT_LOCALE;
    if (theme.isEmpty())
        theme = DEFAULT_THEME;

    const auto str_true(QStringLiteral("true"));
    const auto str_false(QStringLiteral("false"));
    const std::vector<std::pair<const ConfigEntryType, const QString>> entries {
        { ConfigEntryType::FULLSCREEN, fullscreen ? str_true : str_false },
        { ConfigEntryType::LOCALE, locale },
        { ConfigEntryType::THEME, theme },
        { ConfigEntryType::ENABLE_ES2, enable_provider_es2 ? str_true : str_false },
        { ConfigEntryType::ENABLE_STEAM, enable_provider_steam ? str_true : str_false },
    };


    const ConfigEntryMap config_entry_map;
    QTextStream stream(&config_file);
    for (const auto& entry : entries) {
        stream << config_entry_map.type_to_str.at(entry.first)
               << QLatin1String(": ")
               << entry.second
               << '\n';
    }

    qInfo().noquote() << tr_log("Program settings saved");


    using ScriptEvent = ScriptRunner::EventType;
    ScriptRunner::findAndRunScripts(ScriptEvent::CONFIG_CHANGED);
    ScriptRunner::findAndRunScripts(ScriptEvent::SETTINGS_CHANGED);
}

void AppArgs::parse_gamedirs(const std::function<void(const QString&)>& callback)
{
    constexpr int LINE_MAX_LEN = 4096;

    for (QString& path : paths::configDirs()) {
        path += QStringLiteral("/game_dirs.txt");

        QFile config_file(path);
        if (!config_file.open(QFile::ReadOnly | QFile::Text))
            continue;

        QTextStream stream(&config_file);
        QString line;
        while (stream.readLineInto(&line, LINE_MAX_LEN)) {
            if (!line.startsWith('#'))
                callback(line);
        }
    }
}
