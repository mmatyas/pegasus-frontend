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
#include "LocaleUtils.h"
#include "Utils.h"
#include "model/gaming/Collection.h"

#include <QDebug>


namespace model {

CollectionList::CollectionList(QObject* parent)
    : QObject(parent)
    , m_collection_idx(-1)
{
}

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
        qWarning() << tr_log("Invalid collection index #%1").arg(idx);
        return;
    }

    m_collection_idx = idx;
    emit currentChanged();
    emit currentGameChanged();
}

void CollectionList::shiftIndex(IndexShiftDirection dir)
{
    if (m_collections.isEmpty())
        return;

    const int target_idx = shifterFn(dir)(m_collection_idx, m_collections.count());
    setIndex(target_idx);
}

void CollectionList::incrementIndex() {
    shiftIndex(IndexShiftDirection::INCREMENT);
}

void CollectionList::decrementIndex() {
    shiftIndex(IndexShiftDirection::DECREMENT);
}

void CollectionList::incrementIndexNoWrap() {
    shiftIndex(IndexShiftDirection::INCREMENT_NOWRAP);
}

void CollectionList::decrementIndexNoWrap() {
    shiftIndex(IndexShiftDirection::DECREMENT_NOWRAP);
}

QQmlListProperty<Collection> CollectionList::elementsProp()
{
    static constexpr auto count = &listproperty_count<Collection>;
    static constexpr auto at = &listproperty_at<Collection>;

    return {this, &m_collections, count, at};
}

void CollectionList::setModelData(const std::vector<modeldata::Collection>& data)
{
    // TODO: handle the locking and counting during searching

    m_collections.clear();
    m_collection_idx = -1;

    // NOTE: assuming we have less than 2 million games
    int game_count = 0;

    for (const modeldata::Collection& coll : data) {
        game_count += coll.games().size();
        m_collections.append(new model::Collection(&coll, this));

        connect(m_collections.last(), &Collection::currentGameChanged,
                this, &CollectionList::onGameChanged);
        connect(m_collections.last(), &Collection::gameLaunchRequested,
                this, &CollectionList::gameLaunchRequested);
        connect(m_collections.last(), &Collection::gameFavoriteChanged,
                this, &CollectionList::gameFavoriteChanged);
    }
    qInfo().noquote() << tr_log("%1 games found").arg(game_count);

    if (!m_collections.isEmpty()) {
        setIndex(0);
        emit modelChanged();
    }
}

void CollectionList::onGameChanged()
{
    if (sender() == current())
        emit currentGameChanged();
}

} // namespace model
