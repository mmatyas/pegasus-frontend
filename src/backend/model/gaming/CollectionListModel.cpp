// Pegasus Frontend
// Copyright (C) 2017-2022  Mátyás Mustoha
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


#include "CollectionListModel.h"

#include "model/gaming/Assets.h"
#include "model/gaming/Collection.h"
#include "model/gaming/Game.h"


namespace model {
enum CollectionRoles {
    Self = Qt::UserRole,
    Name,
    SortBy,
    ShortName,
    Summary,
    Description,
    Extra,
    Assets,
    Games,
};
using Roles = CollectionRoles;


CollectionListModel::CollectionListModel(QObject* parent)
    : QAbstractListModel(parent)
{}


QHash<int, QByteArray> CollectionListModel::roleNames() const
{
    static QHash<int, QByteArray> ROLE_NAMES {
        { Roles::Self, QByteArrayLiteral("modelData") },
        { Roles::Name, QByteArrayLiteral("name") },
        { Roles::ShortName, QByteArrayLiteral("shortName") },
        { Roles::SortBy, QByteArrayLiteral("sortBy") },
        { Roles::Summary, QByteArrayLiteral("summary") },
        { Roles::Description, QByteArrayLiteral("description") },
        { Roles::Extra, QByteArrayLiteral("extra") },
        { Roles::Assets, QByteArrayLiteral("assets") },
        { Roles::Games, QByteArrayLiteral("games") },
    };
    return ROLE_NAMES;
}


CollectionListModel& CollectionListModel::update(std::vector<model::Collection*>&& entries)
{
    const bool count_changed = m_entries.size() != entries.size();

    beginResetModel();
    m_entries = std::move(entries);
    endResetModel();

    if (count_changed)
        emit countChanged();

    return *this;
}


int CollectionListModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : m_entries.size();
}


QVariant CollectionListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return {};

    model::Collection* const coll_ptr = m_entries.at(index.row());
    const model::Collection& coll = *coll_ptr;
    switch (role) {
        case Roles::Self: return QVariant::fromValue(coll_ptr);
        case Roles::Name: return coll.name();
        case Roles::ShortName: return coll.shortName();
        case Roles::SortBy: return coll.sortBy();
        case Roles::Summary: return coll.summary();
        case Roles::Description: return coll.description();
        case Roles::Extra: return coll.extraMap();
        case Roles::Assets: return QVariant::fromValue(coll.assetsPtr());
        case Roles::Games: return QVariant::fromValue(coll.gameList());
        default: return {};
    }
}


QVariantList CollectionListModel::toVarArray() const
{
    QVariantList varlist;
    varlist.reserve(m_entries.size());
    for (model::Collection* ptr : m_entries)
        varlist.append(QVariant::fromValue(ptr));
    return varlist;
}


model::Collection* CollectionListModel::get(int idx) const
{
    return (0 <= idx && static_cast<size_t>(idx) < m_entries.size())
        ? m_entries.at(idx)
        : nullptr;
}
} // namespace model
