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
#include "utils/StdHelpers.h"

#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QStandardPaths>
#include <QStringBuilder>
#include <array>


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

QString find_assetdir()
{
    const QStringList asset_roots = QStandardPaths::standardLocations(QStandardPaths::GenericCacheLocation);
    for (const QString& root_path : asset_roots) {
        QString assetdir = root_path + QLatin1String("/lutris/");  
        QString banner_subdir = assetdir + QLatin1String("banners/");
        if (QFileInfo::exists(banner_subdir)) 
            return assetdir;
    }
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
            game.assetsMut()
                .add_file(AssetType::UI_STEAMGRID, path)
                .add_file(AssetType::UI_BANNER, path);
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
            game.assetsMut()
                .add_file(AssetType::BACKGROUND, path)
                .add_file(AssetType::POSTER, path)
                .add_file(AssetType::BOX_FRONT, path);
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

    QString assetdir = find_assetdir();
    if (assetdir.isEmpty()) {
        assetdir = datadir;
    }

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
    query.prepare(QLatin1String("SELECT * FROM games WHERE installed = 1"));
    if (!query.exec()) {
        Log::warning(display_name(), LOGMSG("Could not query game data: %1").arg(query.lastError().text()));
        return *this;
    }

    const QSqlRecord record = query.record();
    // TODO: C++20 designated initializers
    const int record_col_id = record.indexOf(QLatin1String("id"));
    const int record_col_slug = record.indexOf(QLatin1String("slug"));
    const int record_col_name = record.indexOf(QLatin1String("name"));
    const int record_col_service = record.indexOf(QLatin1String("service"));
    const int record_col_serviceid = record.indexOf(QLatin1String("service_id"));
    const int record_col_runner = record.indexOf(QLatin1String("runner"));
    const int record_col_steamid = record.indexOf(QLatin1String("steamid"));

    const std::array<int, 3> required_columns {
        record_col_id,
        record_col_slug,
        record_col_name,
    };
    if (VEC_CONTAINS_PRED(required_columns, [](int val){ return val < 0; })) {
        Log::warning(display_name(), LOGMSG("The Lutris database seems to have a different structure than expected"));
        return *this;
    }

    model::Collection& lutris_collection = *sctx.get_or_create_collection(QStringLiteral("Lutris"));

    using QSP = QStandardPaths;
    const QString base_path_banners = assetdir + QLatin1String("banners/");
    const QString base_path_coverart = assetdir + QLatin1String("coverart/");
    const QString base_path_icons = QSP::standardLocations(QSP::GenericDataLocation).constFirst()
        + QLatin1String("/icons/hicolor/128x128/apps/lutris_");

    const QLatin1String STEAM_NAME("steam");
    while (query.next()) {
        const QString id_str = query.value(record_col_id).toString();
        const QString slug = query.value(record_col_slug).toString();
        const QString title = query.value(record_col_name).toString();
        if (id_str.isEmpty() || slug.isEmpty() || title.isEmpty())
            continue;

        QString steamid;

        // A. Try the service + serviceid pair
        const bool uses_service_cols = record_col_service >= 0 && record_col_serviceid >= 0;
        if (uses_service_cols) {
            const QString service = query.value(record_col_service).toString();
            if (service == STEAM_NAME)
                steamid = query.value(record_col_serviceid).toString();
        }

        // B. Try the older runner + steamid pair
        if (steamid.isEmpty()) {
            const bool uses_runner_steamid_cols = record_col_runner >= 0 && record_col_steamid >= 0;
            if (uses_runner_steamid_cols) {
                const QString runner = query.value(record_col_runner).toString();
                if (runner == STEAM_NAME)
                    steamid = query.value(record_col_steamid).toString();
            }
        }

        QString target_uri = !steamid.isEmpty()
            ? QLatin1String("steam:") + steamid
            : QLatin1String("lutris:") + slug;
        model::Collection& target_collection = !steamid.isEmpty()
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
