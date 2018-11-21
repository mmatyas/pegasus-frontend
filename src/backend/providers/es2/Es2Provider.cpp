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


#include "Es2Provider.h"


namespace providers {
namespace es2 {


Es2Provider::Es2Provider(QObject* parent)
    : Provider(parent)
    , systems(this)
    , metadata(this)
{
    connect(&systems, &SystemsParser::gameCountChanged,
            this, &Es2Provider::gameCountChanged);
}

void Es2Provider::findLists(HashMap<QString, modeldata::Game>& games,
                            HashMap<QString, modeldata::Collection>& collections,
                            HashMap<QString, std::vector<QString>>& collection_childs)
{
    systems.find(games, collections, collection_childs);
}

void Es2Provider::findStaticData(HashMap<QString, modeldata::Game>& games,
                                 const HashMap<QString, modeldata::Collection>& collections,
                                 const HashMap<QString, std::vector<QString>>& collection_childs)
{
    metadata.enhance(games, collections, collection_childs);
}

} // namespace es2
} // namespace providers
