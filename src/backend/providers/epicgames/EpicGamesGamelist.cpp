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


#include "EpicGamesGamelist.h"

#include "Log.h"
#include "model/gaming/Game.h"
#include "providers/SearchContext.h"
#include "utils/CommandTokenizer.h"

#include <QDirIterator>
#include <QStringBuilder>


namespace providers {
namespace epicgames {

Gamelist::Gamelist(QString log_tag)
    : m_log_tag(std::move(log_tag))
    , m_name_filters { QStringLiteral("*.item") }
    , m_rx_manifest_catalog_app_name(QStringLiteral(R""("MainGameAppName":\s+"([^"]+)")""))
    , m_rx_manifest_title(QStringLiteral(R""("DisplayName":\s+"([^"]+)")""))
{}

std::tuple<QString, QString> Gamelist::read_manifest_file(const QString& manifest_path) const
{
    QFile manifest(manifest_path);
    if (!manifest.open(QIODevice::ReadOnly | QIODevice::Text)) {
        Log::error(m_log_tag, LOGMSG("Could not open `%1`").arg(manifest_path));
        return {};
    }

    QString catalog_app_name;
    QString title;

    QTextStream stream(&manifest);
    while (!stream.atEnd() && (catalog_app_name.isEmpty() || title.isEmpty())) {
        const QString line = stream.readLine();

        const auto catalog_app_name_match = m_rx_manifest_catalog_app_name.match(line);
        if (catalog_app_name_match.hasMatch()) {
            catalog_app_name = catalog_app_name_match.captured(1);
            continue;
        }

        const auto title_match = m_rx_manifest_title.match(line);
        if (title_match.hasMatch()) {
            title = title_match.captured(1);
            continue;
        }
    }

    return { catalog_app_name, title };
}

// TODO: Simplfy - seems Epic Games only has one manifest dir
HashMap<QString, model::Game*> Gamelist::find_in(
    const QString& epicgames_call,
    const QString& dir_path,
    model::Collection& collection,
    SearchContext& sctx) const
{
    HashMap<QString, model::Game*> result;

    constexpr auto dir_filters = QDir::Files | QDir::Readable | QDir::NoDotAndDotDot;
    constexpr auto dir_flags = QDirIterator::FollowSymlinks;

    QDirIterator dir_it(dir_path, m_name_filters, dir_filters, dir_flags);
    while (dir_it.hasNext()) {
        const QString manifest_path = dir_it.next();
        const QFileInfo finfo = dir_it.fileInfo();

        Log::info(m_log_tag, LOGMSG("Processing `%1`").arg(manifest_path));

        // TODO: C++17
        QString catalog_app_name;
        QString title;
        std::tie(catalog_app_name, title) = read_manifest_file(manifest_path);
        if (catalog_app_name.isEmpty())
            continue;
        if (title.isEmpty())
            title = QLatin1String("Codename ") + catalog_app_name;

        const QString epicgames_uri = QLatin1String("epicgames:") % catalog_app_name;
        model::Game* game_ptr = sctx.game_by_uri(epicgames_uri);
        if (!game_ptr) {
            game_ptr = sctx.create_game_for(collection);
            sctx.game_add_uri(*game_ptr, epicgames_uri);
        }
        sctx.game_add_to(*game_ptr, collection);

        const QString game_url = ::utils::escape_command(
                QLatin1String(" com.epicgames.launcher://apps/") %
                catalog_app_name %
                QLatin1String("?action=launch&silent=true"));

        const QString launch_cmd = epicgames_call % game_url;

        Log::info(m_log_tag, LOGMSG("Setting launch cmd `%1`").arg(launch_cmd));

        (*game_ptr)
            .setTitle(title)
            .setSortBy(title)
            .setLaunchCmd(launch_cmd);

        result.emplace(catalog_app_name, game_ptr);
    }

    return result;
}

} // namespace epicgames
} // namespace providers
