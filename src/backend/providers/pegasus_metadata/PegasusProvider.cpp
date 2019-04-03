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


#include "PegasusProvider.h"

#include "AppSettings.h"
#include "LocaleUtils.h"
#include "Paths.h"
#include "PegasusMetadata.h"
#include "PegasusMedia.h"

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>


namespace {
std::vector<QString> get_game_dirs()
{
    std::vector<QString> game_dirs;

    // DEPRECATED
    const QFileInfo global_finfo(paths::writableConfigDir() + QLatin1String("/global_collection"));
    if (global_finfo.isDir())
        game_dirs.emplace_back(global_finfo.canonicalFilePath());

    AppSettings::parse_gamedirs([&game_dirs](const QString& line){
        const QFileInfo finfo(line);
        if (!finfo.isDir()) {
            qWarning().noquote() << tr_log("Metafiles: game directory `%1` not found, ignored").arg(line);
            return;
        }

        game_dirs.emplace_back(finfo.canonicalFilePath());
    });

    game_dirs.erase(std::unique(game_dirs.begin(), game_dirs.end()), game_dirs.end());
    return game_dirs;
}
} // namespace


namespace providers {
namespace pegasus {

PegasusProvider::PegasusProvider(QObject* parent)
    : PegasusProvider(get_game_dirs(), parent)
{}

PegasusProvider::PegasusProvider(std::vector<QString> game_dirs, QObject* parent)
    : Provider(QStringLiteral("Metafiles"), PROVIDES_GAMES | PROVIDES_ASSETS, parent)
    , m_game_dirs(std::move(game_dirs))
{}

void PegasusProvider::findLists(SearchContext& ctx)
{
    // NOTE: after this call, m_game_dirs also contains the collection directories
    find_in_dirs(m_game_dirs, ctx);
    emit gameCountChanged(static_cast<int>(ctx.games.size()));
}

void PegasusProvider::findStaticData(SearchContext& ctx)
{
    find_assets(ctx.game_root_dirs, ctx.games);
}

} // namespace pegasus
} // namespace providers
