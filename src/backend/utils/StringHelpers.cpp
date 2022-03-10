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


#include "StringHelpers.h"

#include "utils/HashMap.h"
#include <QString>
#include <cctype>
#include <cstring>


namespace utils {
std::string trimmed(const char* const str)
{
    size_t from = 0;
    size_t to = ::strlen(str);
    while (from < to && std::isspace(str[from]))
        from++;
    while (from < to && std::isspace(str[to - 1]))
        to--;
    return std::string(str + from, to - from);
}

bool as_bool(const QString& str, bool& success)
{
    static const HashMap<QString, bool> STR_BOOL_VALS {
        { QStringLiteral("yes"), true },
        { QStringLiteral("on"), true },
        { QStringLiteral("true"), true },
        { QStringLiteral("no"), false },
        { QStringLiteral("off"), false },
        { QStringLiteral("false"), false },
    };

    const auto it = STR_BOOL_VALS.find(str.toLower());
    success = it != STR_BOOL_VALS.cend();
    return success
        ? it->second
        : false;
}
} // namespace utils
