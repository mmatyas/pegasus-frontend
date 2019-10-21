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


#pragma once

#include <algorithm>
#include <vector>


#define VEC_SORT(vec) \
    std::sort(vec.begin(), vec.end())

#define VEC_REMOVE_DUPLICATES(vec) \
    VEC_SORT(vec); \
    vec.erase(std::unique(vec.begin(), vec.end()), vec.end())

#define VEC_REMOVE_VALUE(vec, val) \
    vec.erase(std::remove(vec.begin(), vec.end(), val), vec.end())

#define VEC_REMOVE_IF(vec, unary) \
    vec.erase(std::remove_if(vec.begin(), vec.end(), unary), vec.end())

#define VEC_CONTAINS(vec, val) \
    (std::find(vec.cbegin(), vec.cend(), val) != vec.cend())


template<typename List1, typename List2>
typename List1::iterator vec_append_move(List1& dest, List2&& src)
{
    auto result = dest.insert(
        dest.end(),
        std::make_move_iterator(src.begin()),
        std::make_move_iterator(src.end()));
    src.clear();
    return result;
}

template<typename List1, typename List2>
typename List1::iterator vec_append_copy(List1& dest, const List2& src)
{
    return dest.insert(dest.end(), src.cbegin(), src.cend());
}
