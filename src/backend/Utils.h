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


/// Returns true if the path is an existing regular path (but not embedded)
bool validExtPath(const QString& path);

/// Returns true if the path is an existing regular or embedded file
bool validFile(const QString& path);


/// Mathematical modulo -- the result has the same sign as the divisor
inline int mathMod(int a, int n) { return (a % n + n) % n; }

bool is_str_bool(const QString&);
bool str_to_bool(const QString&, const bool default_val = false);


// Index shifting
enum class IndexShiftDirection : unsigned char {
    INCREMENT,
    INCREMENT_NOWRAP,
    DECREMENT,
    DECREMENT_NOWRAP,
};
const std::function<int(int,int)>& shifterFn(IndexShiftDirection);
