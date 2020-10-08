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
#include "model/gaming/Game.h"
#include "providers/SearchContext.h"
#include "utils/SqliteDb.h"

#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QStringBuilder>


namespace {
constexpr auto MSG_PREFIX = "Lutris:";

QString find_datadir()
{
    using QSP = QStandardPaths;

    for (const QString& commondir : QSP::standardLocations(QSP::GenericDataLocation)) {
        QString lutrisdir = commondir + QLatin1String("/lutris/");
        if (QFileInfo::exists(lutrisdir)) {
            qInfo().noquote() << MSG_PREFIX << tr_log("found data directory: `%1`").arg(lutrisdir);
            return lutrisdir;
        }
    }

    qInfo().noquote() << MSG_PREFIX << tr_log("no installation found");
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
            game.assets().add_file(AssetType::UI_STEAMGRID, path);
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
            game.assets().add_file(AssetType::BACKGROUND, path);
            return;
        }
    }
}

void find_icon_for(model::Game& game, const QString& slug, const QString& base_path)
{
    const QString path = base_path % slug % QLatin1String(".png");
    if (QFileInfo::exists(path))
        game.assets().add_file(AssetType::UI_TILE, path);
}
} // namespace


namespace providers {
namespace lutris {

LutrisProvider::LutrisProvider(QObject* parent)
    : Provider(QLatin1String("lutris"), QStringLiteral("Lutris"), PROVIDES_GAMES | PROVIDES_ASSETS, parent)
{}

Provider& LutrisProvider::findLists(SearchContext& sctx)
{
    const QString datadir = find_datadir();
    if (datadir.isEmpty())
        return *this;

    const QString db_path = datadir + QLatin1String("pga.db");
    if (!QFileInfo::exists(db_path)) {
        qWarning().noquote() << MSG_PREFIX << tr_log("database not found");
        return *this;
    }

    SqliteDb channel(db_path);
    if (!channel.open()) {
        qWarning().noquote() << MSG_PREFIX << tr_log("could not open the database");
        return *this;
    }
    // No entries yet
    if (!channel.hasTable(QStringLiteral("games")))
        return *this;

    QSqlQuery query;
    query.prepare(QLatin1String("SELECT id, slug, name, playtime FROM games"));
    if (!query.exec()) {
        qWarning().noquote() << MSG_PREFIX << query.lastError().text();
        return *this;
    }


    PendingCollection& collection = sctx.get_or_create_collection(QStringLiteral("Lutris"));

    using QSP = QStandardPaths;
    const QString base_path_banners = datadir + QLatin1String("banners/");
    const QString base_path_coverart = datadir + QLatin1String("coverart/");
    const QString base_path_icons = QSP::standardLocations(QSP::GenericDataLocation).first()
        + QLatin1String("/icons/hicolor/128x128/apps/lutris_");


    const size_t game_count_before = sctx.games().size();
    while (query.next()) {
        const QString id_str = query.value(0).toString();
        const QString slug = query.value(1).toString();
        const QString title = query.value(2).toString();
        QString lutris_uri = QLatin1String("lutris:") + slug;

        const PendingGame& entry = sctx.add_or_create_game_from_entry(lutris_uri, collection);
        model::Game& game = entry.inner();
        game.setTitle(title);
        game.setLaunchCmd(QLatin1String("lutris rungameid/") + id_str);

        find_banner_for(game, slug, base_path_banners);
        find_coverart_for(game, slug, base_path_coverart);
        find_icon_for(game, slug, base_path_icons);
    }
    if (game_count_before != sctx.games().size())
        emit gameCountChanged(static_cast<int>(sctx.games().size()));

    return *this;
}

} // namespace lutris
} // namespace providers
