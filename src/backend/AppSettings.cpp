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


#include "AppSettings.h"

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

HashMap<KeyEvent, QVector<int>, EnumHash> default_keymap()
{
    return {
        { KeyEvent::ACCEPT, { Qt::Key_Return, Qt::Key_Enter }},
        { KeyEvent::CANCEL, { Qt::Key_Escape, Qt::Key_Backspace }},
        { KeyEvent::DETAILS, { Qt::Key_I }},
        { KeyEvent::FILTERS, { Qt::Key_F }},
        { KeyEvent::NEXT_PAGE, { Qt::Key_E, Qt::Key_D }},
        { KeyEvent::PREV_PAGE, { Qt::Key_Q, Qt::Key_A }},
        { KeyEvent::PAGE_UP, { Qt::Key_PageUp }},
        { KeyEvent::PAGE_DOWN, { Qt::Key_PageDown }},
    };
}
HashMap<KeyEvent, int, EnumHash> default_gamepadmap()
{
    return {
        { KeyEvent::ACCEPT, static_cast<int>(GamepadKeyId::A) },
        { KeyEvent::CANCEL, static_cast<int>(GamepadKeyId::B) },
        { KeyEvent::DETAILS, static_cast<int>(GamepadKeyId::X) },
        { KeyEvent::FILTERS, static_cast<int>(GamepadKeyId::Y) },
        { KeyEvent::NEXT_PAGE, static_cast<int>(GamepadKeyId::R1) },
        { KeyEvent::PREV_PAGE, static_cast<int>(GamepadKeyId::L1) },
        { KeyEvent::PAGE_UP, static_cast<int>(GamepadKeyId::L2) },
        { KeyEvent::PAGE_DOWN, static_cast<int>(GamepadKeyId::R2) },
    };
}

enum class ConfigEntryCategory : unsigned char {
    GENERAL,
    PROVIDERS,
    KEYS,
};
enum class ConfigEntryGeneralOption : unsigned char {
    PORTABLE,
    SILENT,
    FULLSCREEN,
    LOCALE,
    THEME,
};
struct ConfigEntryMaps {
    using Category = ConfigEntryCategory;
    using GeneralOption = ConfigEntryGeneralOption;

    const HashMap<QString, Category> str_to_category {
        { QStringLiteral("general"), Category::GENERAL },
        { QStringLiteral("providers"), Category::PROVIDERS },
        { QStringLiteral("keys"), Category::KEYS },
    };
    const HashMap<QString, GeneralOption> str_to_general_opt {
        { QStringLiteral("portable"), GeneralOption::PORTABLE },
        { QStringLiteral("silent"), GeneralOption::SILENT },
        { QStringLiteral("fullscreen"), GeneralOption::FULLSCREEN },
        { QStringLiteral("locale"), GeneralOption::LOCALE },
        { QStringLiteral("theme"), GeneralOption::THEME },
    };
    const HashMap<QString, ExtProvider> str_to_extprovider {
        { QStringLiteral("es2"), ExtProvider::ES2 },
        { QStringLiteral("steam"), ExtProvider::STEAM },
        { QStringLiteral("gog"), ExtProvider::GOG },
    };
    const HashMap<QString, KeyEvent> str_to_key_opt {
        { QStringLiteral("accept"), KeyEvent::ACCEPT },
        { QStringLiteral("cancel"), KeyEvent::CANCEL },
        { QStringLiteral("details"), KeyEvent::DETAILS },
        { QStringLiteral("filters"), KeyEvent::FILTERS },
        { QStringLiteral("next-page"), KeyEvent::NEXT_PAGE },
        { QStringLiteral("prev-page"), KeyEvent::PREV_PAGE },
        { QStringLiteral("page-up"), KeyEvent::PAGE_UP },
        { QStringLiteral("page-down"), KeyEvent::PAGE_DOWN },
    };
};

} // namespace


namespace appsettings {
General::General()
    : DEFAULT_LOCALE(QStringLiteral("en"))
    , DEFAULT_THEME(QStringLiteral(":/themes/pegasus-grid/"))
    , portable(false)
    , silent(false)
    , fullscreen(true)
    , locale(DEFAULT_LOCALE)
    , theme(DEFAULT_THEME)
{}


Keys::Keys()
    : m_event_keyboard(default_keymap())
    , m_event_gamepad(default_gamepadmap())
{}
void Keys::add_key(KeyEvent event, int key)
{
    for (auto& entry : m_event_keyboard)
        entry.second.removeOne(key);

    m_event_keyboard.at(event).append(key);
}
void Keys::del_key(KeyEvent event, int key)
{
    m_event_keyboard.at(event).removeOne(key);
}
void Keys::clear(KeyEvent event)
{
    m_event_keyboard.at(event).clear();
}
void Keys::resetAll()
{
    m_event_keyboard = default_keymap();
}
const QVector<int>& Keys::at(KeyEvent event) const {
    return m_event_keyboard.at(event);
}
const QVector<int>& Keys::operator[](KeyEvent event) const {
    return at(event);
}
int Keys::gamepadKey(KeyEvent event) const {
    return m_event_gamepad.at(event);
}


Providers::Providers()
    : m_providers {
        { ExtProvider::ES2, { true } },
        { ExtProvider::STEAM, { true } },
        { ExtProvider::GOG, { true } },
    }
{}
Providers::ExtProviderInfo& Providers::mut(ExtProvider key) {
    return m_providers.at(key);
}
const Providers::ExtProviderInfo& Providers::at(ExtProvider key) const {
    return m_providers.at(key);
}
const Providers::ExtProviderInfo& Providers::operator[](ExtProvider key) const {
    return at(key);
}
} // namespace appsettings


appsettings::General AppSettings::general;
appsettings::Keys AppSettings::keys;
appsettings::Providers AppSettings::ext_providers;


void AppSettings::load_config()
{
    const auto config_path = ::config_path();
    const ConfigEntryMaps maps;


    const auto on_error = [&](const int lineno, const QString msg){
        qWarning().noquote()
            << tr_log("`%1`, line %2: %3").arg(config_path, QString::number(lineno), msg);
    };

    const auto on_attribute = [&](const int lineno, const QString key, const QString val){
        const auto on_error_unknown_opt = [&](){
            on_error(lineno, tr_log("unrecognized option `%1`, ignored").arg(key));
        };

        QStringList sections = key.split('.');
        if (sections.size() < 2) {
            on_error_unknown_opt();
            return;
        }
        const QString category_str = sections.takeFirst();
        const auto category_it = maps.str_to_category.find(category_str);
        if (category_it == maps.str_to_category.cend()) {
            on_error_unknown_opt();
            return;
        }

        const auto on_error_needs_bool = [&](){
            on_error(lineno, tr_log("this option (`%1`) must be a boolean (true/false) value").arg(key));
        };

        switch (category_it->second) {
            case ConfigEntryCategory::GENERAL: {
                const auto option_it = maps.str_to_general_opt.find(sections.constFirst());
                if (option_it == maps.str_to_general_opt.cend()) {
                    on_error_unknown_opt();
                    return;
                }
                switch (option_it->second) {
                    case ConfigEntryGeneralOption::PORTABLE:
                        general.portable = ::str_to_bool(val, general.portable, on_error_needs_bool);
                        break;
                    case ConfigEntryGeneralOption::SILENT:
                        general.silent = ::str_to_bool(val, general.silent, on_error_needs_bool);
                        break;
                    case ConfigEntryGeneralOption::FULLSCREEN:
                        general.fullscreen = ::str_to_bool(val, general.fullscreen, on_error_needs_bool);
                        break;
                    case ConfigEntryGeneralOption::LOCALE:
                        general.locale = val;
                        break;
                    case ConfigEntryGeneralOption::THEME:
                        general.theme = val;
                        break;
                }
                break;
            }
            case ConfigEntryCategory::PROVIDERS: {
                if (sections.size() < 2) {
                    on_error_unknown_opt();
                    return;
                }
                const auto provider_it = maps.str_to_extprovider.find(sections.takeFirst());
                if (provider_it == maps.str_to_extprovider.cend()) {
                    on_error_unknown_opt();
                    return;
                }
                auto& provider = ext_providers.mut(provider_it->second);
                const auto option = sections.takeFirst();
                if (option == QStringLiteral("enabled")) {
                    provider.enabled = ::str_to_bool(val, provider.enabled, on_error_needs_bool);
                }
                break;
            }
            case ConfigEntryCategory::KEYS:
            {
                const auto key_it = maps.str_to_key_opt.find(sections.constFirst());
                if (key_it == maps.str_to_key_opt.cend()) {
                    on_error_unknown_opt();
                    return;
                }

                QVector<int> key_nums;
                const auto key_strs = val.splitRef(',');
                for (QStringRef str : key_strs) {
                    const int key = str.toInt();
                    key_nums << key;
                }
                key_nums.removeAll(0);

                keys.clear(key_it->second);
                for (const int key : key_nums)
                    keys.add_key(key_it->second, key);

                break;
            }
        }
    };

    config::readFile(config_path, on_attribute, on_error);
    qInfo().noquote() << tr_log("Program settings loaded");
}

void AppSettings::save_config()
{
    const auto config_path = ::config_path();

    QFile config_file(config_path);
    if (!config_file.open(QFile::WriteOnly | QFile::Text)) {
        qWarning().noquote() << tr_log("Failed to save program settings to `%1`")
                                .arg(config_path);
        return;
    }


    const auto STR_TRUE(QStringLiteral("true"));
    const auto STR_FALSE(QStringLiteral("false"));
    const auto LINE_TEMPLATE(QStringLiteral("%1.%2: %3\n"));

    QTextStream stream(&config_file);


    // sanity check

    if (general.locale.isEmpty())
        general.locale = general.DEFAULT_LOCALE;
    if (general.theme.isEmpty())
        general.theme = general.DEFAULT_THEME;


    // printing (NOTE: slightly ugly but uses less memory like this)

    ConfigEntryMaps maps;

    HashMap<ConfigEntryCategory, QString, EnumHash> category_to_str;
    for (const auto& entry : maps.str_to_category)
        category_to_str.emplace(entry.second, entry.first);

    // print general options
    {
        using GeneralOption = ConfigEntryGeneralOption;

        HashMap<GeneralOption, QString, EnumHash> general_opt_to_str;
        for (const auto& entry : maps.str_to_general_opt)
            general_opt_to_str.emplace(entry.second, entry.first);

        HashMap<GeneralOption, QString, EnumHash> values {
            { GeneralOption::FULLSCREEN, general.fullscreen ? STR_TRUE : STR_FALSE },
            { GeneralOption::LOCALE, general.locale },
            { GeneralOption::THEME, general.theme },
        };

        for (const auto& entry : values) {
            stream << LINE_TEMPLATE.arg(category_to_str.at(ConfigEntryCategory::GENERAL),
                                        general_opt_to_str.at(entry.first),
                                        entry.second);
        }
    }
    // print provider info
    for (const auto& entry : maps.str_to_extprovider) {
        stream << LINE_TEMPLATE.arg(category_to_str.at(ConfigEntryCategory::PROVIDERS),
                                    entry.first + QStringLiteral(".enabled"),
                                    ext_providers[entry.second].enabled ? STR_TRUE : STR_FALSE);
    }
    // print keyboard config
    for (const auto& entry : maps.str_to_key_opt) {
        QStringList key_strs;
        for (const int key : keys.at(entry.second))
            key_strs << QString::number(key);

        if (key_strs.isEmpty()) // 0 entries will be removed on read
            key_strs << QStringLiteral("0");

        stream << LINE_TEMPLATE.arg(category_to_str.at(ConfigEntryCategory::KEYS),
                                    entry.first,
                                    key_strs.join(','));
    }

    qInfo().noquote() << tr_log("Program settings saved");


    using ScriptEvent = ScriptRunner::EventType;
    ScriptRunner::findAndRunScripts(ScriptEvent::CONFIG_CHANGED);
    ScriptRunner::findAndRunScripts(ScriptEvent::SETTINGS_CHANGED);
}

void AppSettings::parse_gamedirs(const std::function<void(const QString&)>& callback)
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
