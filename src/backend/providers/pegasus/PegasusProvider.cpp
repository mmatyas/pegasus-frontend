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


namespace providers {
namespace pegasus {

PegasusProvider::PegasusProvider(QObject* parent)
    : Provider(parent)
{
    add_game_dir(paths::writableConfigDir() + QStringLiteral("/global_collection"), true);

    AppSettings::parse_gamedirs([this](const QString& line){
        add_game_dir(line);
    });
}

void PegasusProvider::add_game_dir(const QString& dir_path, bool silent)
{
    static constexpr auto MSG_PREFIX = "Collections:";

    const QFileInfo entry(dir_path);
    if (!entry.exists() || !entry.isDir()) {
        if (!silent) {
            qWarning().noquote() << MSG_PREFIX
                << tr_log("game directory `%1` not found, ignored").arg(dir_path);
        }
        return;
    }

    m_game_dirs.emplace_back(entry.canonicalFilePath());
}

void PegasusProvider::findLists(HashMap<QString, modeldata::Game>& games,
                                HashMap<QString, modeldata::Collection>& collections,
                                HashMap<QString, std::vector<QString>>& collection_childs)
{
    m_game_dirs.erase(std::unique(m_game_dirs.begin(), m_game_dirs.end()), m_game_dirs.end());

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
