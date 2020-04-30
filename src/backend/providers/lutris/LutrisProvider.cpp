// Pegasus Frontend
// Copyright (C) 2017-2019  Mátyás Mustoha
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

#include "LocaleUtils.h"
#include "utils/SqliteDb.h"

#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QStringBuilder>


namespace {

QString find_datadir()
{
    using QSP = QStandardPaths;

    for (const QString& commondir : QSP::standardLocations(QSP::GenericDataLocation)) {
        QString lutrisdir = commondir + QLatin1String("/lutris/");
        if (QFileInfo::exists(lutrisdir)) {
            return lutrisdir;
        }
    }

    return {};
}

void find_banner_for(modeldata::Game& game, const QString& slug, const QString& base_path)
{
    const std::array<QLatin1String, 2> exts {
        QLatin1String(".png"),
        QLatin1String(".jpg"),
    };
    for (const QLatin1String& ext : exts) {
        QString path = base_path % slug % ext;
        if (QFileInfo::exists(path)) {
            game.assets.addFileMaybe(AssetType::UI_STEAMGRID, path);
            return;
        }
    }
}

void find_icon_for(modeldata::Game& game, const QString& slug, const QString& base_path)
{
    const QString path = base_path % slug % QLatin1String(".png");
    if (QFileInfo::exists(path))
        game.assets.addFileMaybe(AssetType::UI_TILE, path);
}

void register_game(
    modeldata::Game&& game, QString&& path,
    providers::SearchContext& sctx, std::vector<size_t>& collection_childs)
{
    const size_t game_id = sctx.games.size();
    sctx.path_to_gameid.emplace(std::move(path), game_id);
    sctx.games.emplace(game_id, std::move(game));
    collection_childs.emplace_back(game_id);
}

} // namespace


namespace providers {
namespace lutris {

LutrisProvider::LutrisProvider(QObject* parent)
    : Provider(QLatin1String("lutris"), QStringLiteral("Lutris"), PROVIDES_GAMES | PROVIDES_ASSETS, parent)
{}

void LutrisProvider::findLists(SearchContext& sctx)
{
    const QString datadir = find_datadir();
    if (datadir.isEmpty()) {
        Provider::info(tr_log("no installation found"));
        return;
    }

    Provider::info(tr_log("found data directory: `%1`").arg(datadir));

    const QString db_path = datadir + QLatin1String("pga.db");
    if (!QFileInfo::exists(db_path)) {
        Provider::warn(tr_log("database not found"));
        return;
    }

    SqliteDb channel(db_path);
    if (!channel.open()) {
        Provider::warn(tr_log("could not open the database"));
        return;
    }
    // No entries yet
    if (!channel.hasTable(QStringLiteral("games")))
        return;

    QSqlQuery query;
    query.prepare(QLatin1String("SELECT id, slug, name, playtime FROM games"));
    if (!query.exec()) {
        Provider::warn(query.lastError().text());
        return;
    }


    const QString COLLECTION_TAG(QStringLiteral("Lutris"));
    std::vector<size_t>& collection_childs = sctx.collection_childs[COLLECTION_TAG];
    if (!sctx.collections.count(COLLECTION_TAG))
        sctx.collections.emplace(COLLECTION_TAG, modeldata::Collection(COLLECTION_TAG));

    using QSP = QStandardPaths;
    const QString base_path_banners = datadir + QLatin1String("banners/");
    const QString base_path_icons = QSP::standardLocations(QSP::GenericDataLocation).first()
        + QLatin1String("/icons/hicolor/128x128/apps/lutris_");


    const size_t game_count_before = sctx.games.size();
    while (query.next()) {
        const QString id_str = query.value(0).toString();
        const QString slug = query.value(1).toString();
        const QString title = query.value(2).toString();
        QString protocol_id = QLatin1String("lutris:") + slug;

        modeldata::Game game { QFileInfo(protocol_id) };
        game.title = title;
        game.launch_cmd = QLatin1String("lutris rungameid/") + id_str;

        find_banner_for(game, slug, base_path_banners);
        find_icon_for(game, slug, base_path_icons);

        register_game(std::move(game), std::move(protocol_id), sctx, collection_childs);
    }
    if (game_count_before != sctx.games.size())
        emit gameCountChanged(static_cast<int>(sctx.games.size()));
}

} // namespace lutris
} // namespace providers
