// Pegasus Frontend
// Copyright (C) 2017-2018  Mátyás Mustoha
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

#include <QHash>
#include <QString>
#include <unordered_map>


// std::unordered_map is too verbose
template <typename Key, typename Val, typename Hash = std::hash<Key>>
using HashMap = std::unordered_map<Key, Val, Hash>;

// hash for strings
namespace std {
    template<> struct hash<QString> {
        std::size_t operator()(const QString& s) const {
            return qHash(s);
        }
    };
    template<> struct hash<QLatin1String> {
        std::size_t operator()(const QLatin1String& s) const {
            return qHash(s);
        }
    };
}

// hash for enum classes
struct EnumHash {
    template <typename T>
    std::size_t operator()(T key) const {
        return static_cast<std::size_t>(key);
    }
};
