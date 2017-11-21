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


#include "CollectionList.h"

#include "ListPropertyFn.h"

#include <QDebug>


namespace Types {

CollectionList::CollectionList(QObject* parent)
    : QObject(parent)
    , m_collection_idx(-1)
{
}

CollectionList::~CollectionList() = default;

Collection* CollectionList::current() const
{
    if (m_collection_idx < 0)
        return nullptr;

    Q_ASSERT(m_collection_idx < m_collections.length());
    return m_collections.at(m_collection_idx);
}

void CollectionList::setIndex(int idx)
{
    // Setting the index to a valid value causes changing the current collection
    // and the current game. Setting the index to an invalid value should not
    // change anything.

    if (idx == m_collection_idx)
        return;

    const bool valid_idx = (idx == -1) || (0 <= idx && idx < m_collections.count());
    if (!valid_idx) {
        qWarning() << tr("Invalid collection index #%1").arg(idx);
        return;
    }

    m_collection_idx = idx;
    emit currentChanged();
    emit currentGameChanged();
}

QQmlListProperty<Collection> CollectionList::elementsProp()
{
    static const auto count = &listproperty_count<Collection>;
    static const auto at = &listproperty_at<Collection>;

    return {this, &m_collections, count, at};
}

void CollectionList::onScanComplete()
{
    // TODO: handle the locking and counting during searching


    // NOTE: `tr` (see below) uses `int`; assuming we have
    //       less than 2 million games, it will be enough
    int game_count = 0;

    for (Collection* const coll : m_collections) {
        coll->setParent(this);

        connect(coll, &Collection::currentGameChanged,
                this, &CollectionList::onGameChanged);

        Types::GameList& gamelist = coll->gameListMut();
        gamelist.lockGameList();
        game_count += gamelist.allGames().count();
    }
    qInfo().noquote() << tr("%n games found", "", game_count);

    if (!m_collections.isEmpty()) {
        setIndex(0);
        emit countChanged();
    }
}

void CollectionList::onGameChanged()
{
    if (sender() == current())
        emit currentGameChanged();
}

} // namespace Types
