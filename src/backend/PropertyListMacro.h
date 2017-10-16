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


#pragma once


/// Generates a property list getter for the provided class field (a container)
#define PROPERTYLIST_GETTER(field) \
    using PointedType = std::remove_pointer<decltype(field)::value_type>::type; \
    PROPERTYLIST_GETTER_SPEC(field, PointedType)

/// Generates a property list getter for the provided field (a container),
/// that contains pointers to PointedType
#define PROPERTYLIST_GETTER_SPEC(field, PointedType) \
    static const auto count = [](QQmlListProperty<PointedType>* p) { \
        return reinterpret_cast<decltype(this)>(p->data)->field.count(); \
    }; \
    static const auto at = [](QQmlListProperty<PointedType>* p, int idx) { \
        return reinterpret_cast<decltype(this)>(p->data)->field.at(idx); \
    }; \
    \
    return {this, this, count, at};
