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


#pragma once

#include <QModelIndex>
#include <QObject>
#include <QVariantList>
#include <vector>


namespace model {
namespace utils {

template<typename M, typename T>
void update(M* const self, std::vector<T*>& stored_entries, std::vector<T*>&& new_entries)
{
    const bool count_changed = stored_entries.size() != new_entries.size();

    for (T* entry : stored_entries)
        QObject::disconnect(entry, nullptr, self, nullptr);

    stored_entries = std::move(new_entries);

    for (T* entry : stored_entries)
        self->connectEntry(entry);

    if (count_changed)
        emit self->countChanged();
}


template<typename T>
int rowCount(const QModelIndex& parent, const std::vector<T*>& entries)
{
    return parent.isValid() ? 0 : entries.size();
}


template<typename T>
QVariantList toVarArray(const std::vector<T*>& entries)
{
    QVariantList varlist;
    varlist.reserve(entries.size());
    for (T* ptr : entries)
        varlist.append(QVariant::fromValue(ptr));
    return varlist;
}


template<typename T>
T* get(const std::vector<T*>& entries, int idx)
{
    return (0 <= idx && static_cast<size_t>(idx) < entries.size())
        ? entries.at(idx)
        : nullptr;
}

} // namespace utils
} // namespace model
