// Pegasus Frontend
// Copyright (C) 2018  Mátyás Mustoha
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


#include "IndexShifter.h"

#include "utils/HashMap.h"
#include "utils/MathMod.h"


const std::function<int(int,int)>& shifterFn(IndexShiftDirection direction)
{
    static const HashMap<IndexShiftDirection, std::function<int(int,int)>, EnumHash> fn_table {
        { IndexShiftDirection::INCREMENT,        [](int idx, int count){ return mathMod(idx + 1, count); } },
        { IndexShiftDirection::DECREMENT,        [](int idx, int count){ return mathMod(idx - 1, count); } },
        { IndexShiftDirection::INCREMENT_NOWRAP, [](int idx, int count){ return std::min(idx + 1, count - 1); } },
        { IndexShiftDirection::DECREMENT_NOWRAP, [](int idx, int)      { return std::max(idx - 1, 0); } },
    };
    return fn_table.at(direction);
}
