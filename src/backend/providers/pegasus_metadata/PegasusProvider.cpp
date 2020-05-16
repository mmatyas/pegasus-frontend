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
#include "providers/SearchContext.h"

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
    : Provider(QLatin1String("pegasus_metafiles"), QStringLiteral("Metafiles"), INTERNAL | PROVIDES_GAMES | PROVIDES_ASSETS, parent)
{}

Provider& PegasusProvider::load() {
    return load_with_gamedirs(get_game_dirs());
}
Provider& PegasusProvider::load_with_gamedirs(std::vector<QString> game_dirs) {
    m_game_dirs = std::move(game_dirs);
    return *this;
}
Provider& PegasusProvider::unload() {
    m_game_dirs.clear();
    return *this;
}

Provider& PegasusProvider::findLists(SearchContext& sctx)
{
    // NOTE: after this call, m_game_dirs also contains the collection directories
    find_in_dirs(m_game_dirs, sctx);
    emit gameCountChanged(static_cast<int>(sctx.games().size()));
    return *this;
}

Provider& PegasusProvider::findStaticData(SearchContext& sctx)
{
    find_assets(sctx.game_root_dirs(), sctx);
    return *this;
}

} // namespace pegasus
} // namespace providers
