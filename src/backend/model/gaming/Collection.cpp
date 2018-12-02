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

#include "LocaleUtils.h"
#include "model/ListPropertyFn.h"
#include "utils/SortGames.h"

#include <QDebug>


namespace model {

Collection::Collection(modeldata::Collection collection, QObject* parent)
    : QObject(parent)
    , m_collection(std::move(collection))
    , m_default_assets(&m_collection.default_assets, this)
{}

void Collection::setGameList(QVector<Game*> games)
{
    m_games = std::move(games);
    sort_games(m_games);

    emit gamelistChanged();
}

QQmlListProperty<Game> Collection::qmlGames()
{
    static constexpr auto count = &listproperty_count<Game>;
    static constexpr auto at = &listproperty_at<Game>;

    return {this, &m_games, count, at};
}

} // namespace model
