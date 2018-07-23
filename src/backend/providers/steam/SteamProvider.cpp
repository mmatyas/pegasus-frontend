// Pegasus Frontend
// Copyright (C) 2017  Mátyás Mustoha
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


namespace providers {
namespace steam {

SteamProvider::SteamProvider(QObject* parent)
    : Provider(parent)
    , gamelist(this)
    , metadata(this)
{
    connect(&gamelist, &Gamelist::gameCountChanged,
            this, &SteamProvider::gameCountChanged);
}

void SteamProvider::findLists(HashMap<QString, modeldata::Game>& games,
                              HashMap<QString, modeldata::Collection>& collections,
                              HashMap<QString, std::vector<QString>>& collection_childs)
{
    gamelist.find(games, collections, collection_childs);
}

void SteamProvider::findStaticData(HashMap<QString, modeldata::Game>& games,
                                   const HashMap<QString, modeldata::Collection>& collections,
                                   const HashMap<QString, std::vector<QString>>& collection_childs)
{
    metadata.enhance(games, collections, collection_childs);
}

} // namespace steam
} // namespace providers
