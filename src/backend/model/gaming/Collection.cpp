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


#include "Collection.h"

#include "Assets.h"
#include "Game.h"


namespace model {

CollectionData::CollectionData(QString new_name)
    : name(std::move(new_name))
    , sort_by(name)
    , m_short_name(name.toLower())
{}

void CollectionData::set_short_name(const QString& name)
{
    Q_ASSERT(!name.isEmpty());
    m_short_name = name.toLower();
}

Collection::Collection(QString name, QObject* parent)
    : QObject(parent)
    , m_games(new QQmlObjectListModel<model::Game>(this))
    , m_data(std::move(name))
    , m_assets(new model::Assets(this))
{}

Collection& Collection::setGames(std::vector<model::Game*>&& games)
{
    std::sort(games.begin(), games.end(), model::sort_games);

    QVector<model::Game*> modelvec;
    modelvec.reserve(games.size());
    std::move(games.begin(), games.end(), std::back_inserter(modelvec));

    m_games->append(std::move(modelvec));
    return *this;
}

bool sort_collections(const model::Collection* const a, const model::Collection* const b) {
    return QString::localeAwareCompare(a->sortBy(), b->sortBy()) < 0;
}
} // namespace model
