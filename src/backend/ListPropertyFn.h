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

#include <QQmlListProperty>


template<typename PointedType>
static int listproperty_count(QQmlListProperty<PointedType>* p)
{
    return reinterpret_cast<const QVector<PointedType*>* const>(p->data)->count();
}

template<typename PointedType>
static PointedType* listproperty_at(QQmlListProperty<PointedType>* p, int idx)
{
    return reinterpret_cast<const QVector<PointedType*>* const>(p->data)->at(idx);
}

// listproperty functions for STL containers

template<typename T>
static int listproperty_stl_count(QQmlListProperty<T>* p)
{
    return static_cast<int>(
        reinterpret_cast<const std::vector<T>* const>(p->data)
        ->size()
    );
}

template<typename T>
static T* listproperty_stl_at(QQmlListProperty<T>* p, int idx)
{
    return reinterpret_cast<const std::vector<T>* const>(p->data)
        ->at(static_cast<size_t>(idx)
    );
}
