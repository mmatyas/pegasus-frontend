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

#include <functional>

class QString;
class QStringList;


/// Returns true if the path is an existing regular file or directory
bool validPath(const QString& path);

/// Returns true if the path is an existing regular file or an embedded resource
bool validFileQt(const QString& path);

/// Returns $PEGASUS_HOME if defined, or $HOME if defined,
/// otherwise QDir::homePath().
QString homePath();

/// Returns the directory paths where config files may be located
QStringList configDirPaths();


/// Mathematical modulo -- the result has the same sign as the divisor
inline int mathMod(int a, int n) { return (a % n + n) % n; }


// hash for enum classes
struct EnumClassHash {
    template <typename T>
    std::size_t operator()(T key) const {
        return static_cast<std::size_t>(key);
    }
};

// Index shifting
enum class IndexShiftDirection : unsigned char {
    INCREMENT,
    INCREMENT_NOWRAP,
    DECREMENT,
    DECREMENT_NOWRAP,
};
const std::function<int(int,int)>& shifterFn(IndexShiftDirection);
