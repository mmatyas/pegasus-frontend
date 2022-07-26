// Pegasus Frontend
// Copyright (C) 2017-2022  Mátyás Mustoha
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


#include "EpicProvider.h"

#include "Log.h"
#include "model/gaming/Game.h"
#include "providers/SearchContext.h"
#include "utils/CommandTokenizer.h"
#include "utils/PathTools.h"

#include <QProcessEnvironment>
#include <QStringBuilder>

#include <QDir>
#include <QDirIterator>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>


namespace {
QString find_manifests_dir()
{
    const auto env = QProcessEnvironment::systemEnvironment();
    const QString env_programdata = env.value(QStringLiteral("PROGRAMDATA"));
    if (env_programdata.isEmpty())
        return {};

    const QString expected_path = env_programdata % QStringLiteral("/Epic/EpicGamesLauncher/Data/Manifests");
    if (!QFileInfo::exists(expected_path))
        return {};

    return expected_path;
}


QString escaped_qt_path(const QString& raw_path)
{
    return ::utils::escape_command(QDir::cleanPath(raw_path));
}


struct Manifest {
    QString uri;
    QString title;
    QString cmd;
    QString workdir;
};

Manifest load_manifest(const QString& manifest_path, const QString& log_tag)
{
    QFile file(manifest_path);
    if (!file.open(QIODevice::ReadOnly)) {
        Log::warning(log_tag, LOGMSG("Could not open `%1`").arg(::pretty_path(manifest_path)));
        return {};
    }

    QJsonParseError json_error;
    const QJsonDocument json_doc = QJsonDocument::fromJson(file.readAll(), &json_error);
    if (json_error.error != QJsonParseError::NoError) {
        Log::warning(log_tag, LOGMSG("Could not read `%1` as a JSON file").arg(::pretty_path(manifest_path)));
        Log::warning(log_tag, json_error.errorString());
        return {};
    }

    const QJsonObject json_root = json_doc.object();
    const QString self_name = json_root[QLatin1String("AppName")].toString();
    const QString parent_name = json_root[QLatin1String("MainGameAppName")].toString();
    if (self_name.isEmpty())
        return {};

    const bool is_addon = self_name != parent_name;
    if (is_addon)
        return {};

    bool is_game = false;
    const QJsonArray app_categories = json_root[QLatin1String("AppCategories")].toArray();
    for (const QJsonValue& app_cat : app_categories) {
        if (app_cat.toString() == QLatin1String("games")) {
            is_game = true;
            break;
        }
    }
    if (!is_game)
        return {};

    const QString display_name = json_root[QLatin1String("DisplayName")].toString(self_name);
    const QString launch_wrapper = json_root[QLatin1String("LaunchCommand")].toString();
    const QString launch_exe = json_root[QLatin1String("LaunchExecutable")].toString();
    const QString launch_dir = json_root[QLatin1String("InstallLocation")].toString();
    if (launch_exe.isEmpty() || launch_dir.isEmpty())
        return {};

    const QString launch_exe_abs_path = launch_dir % QDir::separator() % launch_exe;
    const QString launch_cmd = QStringLiteral("%1 %2")
        .arg(escaped_qt_path(launch_wrapper), escaped_qt_path(launch_exe_abs_path))
        .trimmed();

    const QString game_uri = QStringLiteral("epic:") + self_name;
    return {
        game_uri,
        display_name,
        launch_cmd,
        launch_dir,
    };
}
} // namespace


namespace providers {
namespace epic {

EpicProvider::EpicProvider(QObject* parent)
    : Provider(QLatin1String("epic"), QStringLiteral("Epic Games"), parent)
{}

Provider& EpicProvider::run(SearchContext& sctx)
{
    const QString mainfests_dir = find_manifests_dir();
    if (mainfests_dir.isEmpty()) {
        Log::info(display_name(), LOGMSG("No installation found"));
        return *this;
    }

    Log::info(display_name(), LOGMSG("Loading Epic Games manifests from `%1`").arg(::pretty_path(mainfests_dir)));

    model::Collection& collection = *sctx.get_or_create_collection(QStringLiteral("Epic Games"));


    constexpr auto dir_filters = QDir::Files | QDir::Readable | QDir::NoDotAndDotDot;
    constexpr auto dir_flags = QDirIterator::FollowSymlinks;

    QDirIterator dir_it(mainfests_dir, { QStringLiteral("*.item") }, dir_filters, dir_flags);
    while (dir_it.hasNext()) {
        const QString manifest_path = dir_it.next();
        const Manifest info = load_manifest(manifest_path, display_name());
        if (info.uri.isEmpty())
            continue;

        model::Game* game_ptr = sctx.game_by_uri(info.uri);
        if (!game_ptr) {
            game_ptr = sctx.create_game_for(collection);
            sctx.game_add_uri(*game_ptr, info.uri);
        }
        sctx.game_add_to(*game_ptr, collection);

        (*game_ptr)
            .setTitle(info.title)
            .setLaunchCmdBasedir(info.workdir)
            .setLaunchCmd(info.cmd);
    }

    return *this;
}

} // namespace epic
} // namespace providers
