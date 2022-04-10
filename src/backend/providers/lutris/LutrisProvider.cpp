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


#include "LutrisProvider.h"

#include "Log.h"
#include "Paths.h"
#include "model/gaming/Assets.h"
#include "model/gaming/Game.h"
#include "providers/SearchContext.h"
#include "utils/SqliteDb.h"

#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QStringBuilder>


namespace {
QString find_datadir()
{
    const QStringList data_roots = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
    for (const QString& root_path : data_roots) {
        QString lutrisdir = root_path + QLatin1String("/lutris/");
        if (QFileInfo::exists(lutrisdir))
            return lutrisdir;
    }

#if defined(PEGASUS_INSIDE_FLATPAK)
    QString user_root = paths::homePath() + QLatin1String("/.local/share/lutris/");
    if (QFileInfo::exists(user_root))
        return user_root;
#endif

    return {};
}

void find_banner_for(model::Game& game, const QString& slug, const QString& base_path)
{
    const std::array<QLatin1String, 2> exts {
        QLatin1String(".png"),
        QLatin1String(".jpg"),
    };
    for (const QLatin1String& ext : exts) {
        QString path = base_path % slug % ext;
        if (QFileInfo::exists(path)) {
            game.assetsMut().add_file(AssetType::UI_STEAMGRID, path);
            return;
        }
    }
}

void find_coverart_for(model::Game& game, const QString& slug, const QString& base_path)
{
    const std::array<QLatin1String, 2> exts {
        QLatin1String(".png"),
        QLatin1String(".jpg"),
    };
    for (const QLatin1String& ext : exts) {
        QString path = base_path % slug % ext;
        if (QFileInfo::exists(path)) {
            game.assetsMut().add_file(AssetType::BACKGROUND, path);
            return;
        }
    }
}

void find_icon_for(model::Game& game, const QString& slug, const QString& base_path)
{
    const QString path = base_path % slug % QLatin1String(".png");
    if (QFileInfo::exists(path))
        game.assetsMut().add_file(AssetType::UI_TILE, path);
}
} // namespace


namespace providers {
namespace lutris {

LutrisProvider::LutrisProvider(QObject* parent)
    : Provider(QLatin1String("lutris"), QStringLiteral("Lutris"), parent)
{}

Provider& LutrisProvider::run(SearchContext& sctx)
{
    const QString datadir = find_datadir();
    if (datadir.isEmpty()) {
        Log::info(display_name(), LOGMSG("No installation found"));
        return *this;
    }
    Log::info(display_name(), LOGMSG("Found data directory: `%1`").arg(datadir));

    const QString db_path = datadir + QLatin1String("pga.db");
    if (!QFileInfo::exists(db_path)) {
        Log::warning(display_name(), LOGMSG("Database not found"));
        return *this;
    }

    SqliteDb channel(db_path);
    if (!channel.open()) {
        Log::warning(display_name(), LOGMSG("Could not open the database"));
        return *this;
    }
    // No entries yet
    if (!channel.hasTable(QStringLiteral("games")))
        return *this;

    QSqlQuery query;
    query.prepare(QLatin1String("SELECT id, slug, name, playtime, runner, steamid FROM games"));
    if (!query.exec()) {
        Log::warning(display_name(), query.lastError().text());
        return *this;
    }


    model::Collection& lutris_collection = *sctx.get_or_create_collection(QStringLiteral("Lutris"));

    using QSP = QStandardPaths;
    const QString base_path_banners = datadir + QLatin1String("banners/");
    const QString base_path_coverart = datadir + QLatin1String("coverart/");
    const QString base_path_icons = QSP::standardLocations(QSP::GenericDataLocation).constFirst()
        + QLatin1String("/icons/hicolor/128x128/apps/lutris_");

    const QLatin1String STEAM_RUNNER("steam");
    while (query.next()) {
        const QString id_str = query.value(0).toString();
        const QString slug = query.value(1).toString();
        const QString title = query.value(2).toString();

        const QString runner = query.value(4).toString();
        const QString steamid = query.value(5).toString();
        const bool uses_steam = runner == STEAM_RUNNER && !steamid.isEmpty();

        QString target_uri = uses_steam
            ? QLatin1String("steam:") + steamid
            : QLatin1String("lutris:") + slug;
        model::Collection& target_collection = uses_steam
            ? *sctx.get_or_create_collection(QStringLiteral("Steam"))
            : lutris_collection;

        model::Game* game_ptr = sctx.game_by_uri(target_uri);
        if (!game_ptr) {
            game_ptr = sctx.create_game_for(target_collection);
            sctx.game_add_uri(*game_ptr, target_uri);
        }

        model::Game& game = *game_ptr;
        sctx.game_add_to(game, target_collection);

        game.setTitle(title);
        if (game.launchCmd().isEmpty())
            game.setLaunchCmd(QLatin1String("lutris rungameid/") + id_str);

        find_banner_for(game, slug, base_path_banners);
        find_coverart_for(game, slug, base_path_coverart);
        find_icon_for(game, slug, base_path_icons);
    }

    return *this;
}

} // namespace lutris
} // namespace providers
