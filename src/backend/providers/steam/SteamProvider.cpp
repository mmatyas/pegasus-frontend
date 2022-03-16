// Pegasus Frontend
// Copyright (C) 2017-2020  Mátyás Mustoha
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


#include "SteamProvider.h"

#include "Log.h"
#include "Paths.h"
#include "providers/ProviderUtils.h"
#include "providers/SearchContext.h"
#include "providers/steam/SteamGamelist.h"
#include "providers/steam/SteamMetadata.h"
#include "utils/StdHelpers.h"

#include <QDir>
#include <QSettings>
#include <QStandardPaths>
#include <QStringBuilder>
#include <QTextStream>


namespace {
QString find_steam_datadir()
{
    std::vector<QString> possible_dirs;

#ifdef Q_OS_WIN
    const QSettings reg_base(QStringLiteral("HKEY_CURRENT_USER\\Software\\Valve\\Steam"), QSettings::NativeFormat);
    const QString reg_value = reg_base.value(QLatin1String("SteamPath")).toString();
    if (!reg_value.isEmpty())
        possible_dirs.emplace_back(reg_value + QChar('/'));
#endif

#ifdef Q_OS_LINUX
    // Linux: Prefer Flatpak-Steam if available
    possible_dirs.emplace_back(providers::steam_flatpak_data_dir());
#endif // linux

#ifdef Q_OS_UNIX
    // Linux: ~/.local/share/Steam
    // macOS: ~/Library/Application Support/Steam
    const QStringList std_paths = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
    for (const QString& dir : std_paths)
        possible_dirs.emplace_back(dir + QLatin1String("/Steam/"));

#ifdef Q_OS_LINUX
    // in addition on Linux, ~/.steam/steam
    possible_dirs.emplace_back(paths::homePath() + QLatin1String("/.steam/steam/"));
#endif // linux
#endif // unix

    for (const QString& dir_path : possible_dirs) {
        if (QFileInfo::exists(dir_path))
            return dir_path;
    }

    return {};
}

void find_installdirs_in_vdf(
    const QString& log_tag,
    const QString& vdf_path,
    const std::vector<QString>& entry_patterns,
    std::vector<QString>& installdirs)
{
    QFile vdf(vdf_path);
    if (!vdf.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    Log::info(log_tag, LOGMSG("Found `%1`").arg(vdf_path));

    std::vector<QRegularExpression> entry_regexes;
    entry_regexes.reserve(entry_patterns.size());
    for (const QString& pattern : entry_patterns)
        entry_regexes.emplace_back(pattern);

    QTextStream stream(&vdf);
    stream.setCodec("UTF-8");

    while (!stream.atEnd()) {
        const QString line = stream.readLine();
        for (const QRegularExpression& entry_regex : entry_regexes) {
            const auto match = entry_regex.match(line);
            if (match.hasMatch()) {
                QString path = match.captured(1) % QLatin1String("/steamapps/");
                installdirs.emplace_back(std::move(path));
                break;
            }
        }
    }
}

std::vector<QString> find_steam_installdirs(const QString& log_tag, const QString& steam_datadir)
{
    std::vector<QString> installdirs;
    Q_ASSERT(steam_datadir.endsWith(QChar('/')));
    installdirs.emplace_back(steam_datadir + QLatin1String("steamapps/"));

    const QString config_path = steam_datadir + QLatin1String("config/config.vdf");
    const std::vector<QString> config_pattern {
        QStringLiteral(R""("BaseInstallFolder_\d+"\s+"([^"]+)")""),
    };
    find_installdirs_in_vdf(log_tag, config_path, config_pattern, installdirs);

    const QString folderlist_path = installdirs.front() + QLatin1String("libraryfolders.vdf");
    const std::vector<QString> folderlist_patterns {
        QStringLiteral(R""("[1-7]"\s+"([^"]+)")""),
        QStringLiteral(R""("path"\s+"([^"]+)")""),
    };
    find_installdirs_in_vdf(log_tag, folderlist_path, folderlist_patterns, installdirs);

    VEC_REMOVE_DUPLICATES(installdirs);
    VEC_REMOVE_IF(installdirs, [](const QString& path) { return !QFileInfo::exists(path); });
    return installdirs;
}

void fill_metadata_from_cache(HashMap<QString, model::Game*>& appid_game_map, const providers::steam::Metadata& metahelper)
{
    std::vector<QString> found_appids;
    found_appids.reserve(appid_game_map.size());

    // TODO: C++17
    for (const auto& entry : appid_game_map) {
        const QString& appid = entry.first;
        model::Game& game = *entry.second;

        const bool found = metahelper.fill_from_cache(appid, game);
        if (found)
            found_appids.emplace_back(appid);
    }

    for (const QString& appid : found_appids)
        appid_game_map.erase(appid);
}

void fill_metadata_from_network(
    const HashMap<QString, model::Game*>& appid_game_map,
    const providers::steam::Metadata& metahelper,
    providers::SearchContext& sctx)
{
    if (appid_game_map.empty())
        return;

    if (!sctx.has_network())
        return;

    // TODO: C++17
    for (const auto& entry : appid_game_map) {
        const QString& appid = entry.first;
        model::Game* const game = entry.second;
        metahelper.fill_from_network(appid, *game, sctx);
    }
}
} // namespace


namespace providers {
namespace steam {

SteamProvider::SteamProvider(QObject* parent)
    : Provider(QLatin1String("steam"), QStringLiteral("Steam"), parent)
{}

Provider& SteamProvider::run(SearchContext& sctx)
{
    const QString steam_call = providers::find_steam_call();
    Q_ASSERT(!steam_call.isEmpty());


    const QString steamdir_path = find_steam_datadir();
    if (steamdir_path.isEmpty()) {
        Log::info(display_name(), LOGMSG("No installation found"));
        return *this;
    }

    Log::info(display_name(), LOGMSG("Found installation at `%1`").arg(steamdir_path));

    std::vector<QString> installdirs = find_steam_installdirs(display_name(), steamdir_path);
    if (installdirs.empty()) {
        Log::warning(display_name(), LOGMSG("No game directories found"));
        return *this;
    }


    model::Collection& collection = *sctx.get_or_create_collection(QStringLiteral("Steam"));

    const float progress_step = 1.f / installdirs.size();
    float progress = 0.f;

    const Gamelist gamehelper(display_name());

    HashMap<QString, model::Game*> appid_game_map;
    for (const QString& installdir : installdirs) {
        HashMap<QString, model::Game*> local_games = gamehelper.find_in(steam_call, installdir, collection, sctx);

        appid_game_map.insert(
            std::make_move_iterator(local_games.begin()),
            std::make_move_iterator(local_games.end()));

        progress += progress_step;
        emit progressChanged(progress);
    }

    Log::info(display_name(), LOGMSG("%1 games found").arg(QString::number(appid_game_map.size())));
    if (appid_game_map.empty())
        return *this;


    const Metadata metahelper(display_name());
    fill_metadata_from_cache(appid_game_map, metahelper);
    fill_metadata_from_network(appid_game_map, metahelper, sctx);

    return *this;
}

} // namespace steam
} // namespace providers
