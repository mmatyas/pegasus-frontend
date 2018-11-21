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


#include "PegasusProvider.h"

#include "AppSettings.h"
#include "LocaleUtils.h"
#include "Paths.h"

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>


namespace {
std::vector<QString> get_game_dirs()
{
    std::vector<QString> game_dirs;

    const QString global_dir = paths::writableConfigDir() + QStringLiteral("/global_collection");
    const QFileInfo global_finfo(global_dir);
    if (global_finfo.isDir())
        game_dirs.emplace_back(global_finfo.canonicalFilePath());

    constexpr auto MSG_PREFIX = "Collections:";
    AppSettings::parse_gamedirs([&game_dirs](const QString& line){
        const QFileInfo finfo(line);
        if (!finfo.isDir()) {
            qWarning().noquote() << MSG_PREFIX << tr_log("game directory `%1` not found, ignored").arg(line);
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
    : Provider(parent)
    , m_game_dirs(std::move(game_dirs))
{}

void PegasusProvider::findLists(HashMap<QString, modeldata::Game>& games,
                                HashMap<QString, modeldata::Collection>& collections,
                                HashMap<QString, std::vector<QString>>& collection_childs)
{
    collection_finder.find_in_dirs(m_game_dirs, games, collections, collection_childs,
                                   [this](int game_count){ emit gameCountChanged(game_count); });
}

void PegasusProvider::findStaticData(HashMap<QString, modeldata::Game>& games,
                                     const HashMap<QString, modeldata::Collection>& collections,
                                     const HashMap<QString, std::vector<QString>>& collection_childs)
{
    metadata_finder.enhance_in_dirs(m_game_dirs, games, collections, collection_childs);
}

} // namespace pegasus
} // namespace providers
