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


#include "Collection.h"


namespace model {

CollectionData::CollectionData(QString new_name)
    : name(std::move(new_name))
    , sort_name(name)
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
    , m_data(name)
    , m_assets(new model::Assets(this))
{}

bool sort_collections(const model::Collection* const a, const model::Collection* const b) {
    return QString::localeAwareCompare(a->sortName(), b->sortName()) < 0;
};
} // namespace model
