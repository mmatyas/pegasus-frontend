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


#pragma once

#include "utils/HashMap.h"
#include "utils/NoCopyNoMove.h"

#include <QString>
#include <functional>


// A class is used in order to free the hashmap after parsing
// TODO: use a static constexpr hashmap
class StrBoolConverter {
public:
    StrBoolConverter();
    NO_COPY_NO_MOVE(StrBoolConverter)

    bool to_bool(const QString& str, bool& success) const;

private:
    const HashMap<QString, const bool> m_strmap;
};
