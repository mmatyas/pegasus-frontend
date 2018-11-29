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

#include "model/ListPropertyFn.h"
#include "LocaleUtils.h"
#include "model/gaming/Collection.h"

#include <QDebug>


namespace {

void sort_collections(QVector<model::Collection*>& collections)
{
    std::sort(collections.begin(), collections.end(),
        [](const model::Collection* const a, const model::Collection* const b) {
            return QString::localeAwareCompare(a->data().name(), b->data().name()) < 0;
        }
    );
}

} // namespace


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
        qWarning().noquote() << tr_log("Invalid collection index #%1").arg(idx);
        return;
    }

    m_collection_idx = idx;
    emit currentChanged();
}

QQmlListProperty<Collection> CollectionList::elementsProp()
{
    static constexpr auto count = &listproperty_count<Collection>;
    static constexpr auto at = &listproperty_at<Collection>;

    return {this, &m_collections, count, at};
}

void CollectionList::setModelData(const QVector<Collection*>& collections)
{
    Q_ASSERT(m_collections.isEmpty());
    Q_ASSERT(m_collection_idx == -1);

    m_collections = collections;
    sort_collections(m_collections);

    for (Collection* const coll : m_collections)
        coll->setParent(this);

    if (!m_collections.isEmpty()) {
        setIndex(0);
        emit modelChanged();
    }
}

} // namespace model
