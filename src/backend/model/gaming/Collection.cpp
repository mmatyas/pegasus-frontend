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


#include "Collection.h"


namespace model {

Collection::Collection(modeldata::Collection collection, QObject* parent)
    : QObject(parent)
    , m_games(this)
    , m_collection(std::move(collection))
    , m_default_assets(&m_collection.default_assets, this)
{}

void Collection::setGameList(QVector<Game*> games)
{
    m_games.clear();
    m_games.append(std::move(games));
}

} // namespace model
