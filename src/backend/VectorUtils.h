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

#include <algorithm>
#include <vector>


/// Returns the iterator pointing to [item], or [vec].cend()
template<typename T>
typename std::vector<T>::const_iterator find(const std::vector<T>& vec, const T& item) {
    return std::find(vec.cbegin(), vec.cend(), item);
}

/// Returns true if the vector contains the item
template<typename T>
bool contains(const std::vector<T>& vec, const T& item) {
    return find(vec, item) != vec.cend();
}

/// Appends [source] to the end of [target]
template<typename T>
void append(std::vector<T>& target, std::vector<T>& source) {
    target.insert(target.end(), source.begin(), source.end());
}
