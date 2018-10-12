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


#include "StrBoolConverter.h"


StrBoolConverter::StrBoolConverter()
    : m_strmap {
        { QStringLiteral("yes"), true },
        { QStringLiteral("on"), true },
        { QStringLiteral("true"), true },
        { QStringLiteral("no"), false },
        { QStringLiteral("off"), false },
        { QStringLiteral("false"), false },
    }
{}

bool StrBoolConverter::isBool(const QString& str) const
{
    return m_strmap.count(str.toLower());
}

bool StrBoolConverter::toBool(const QString& str, const bool default_val,
                              const std::function<void()>& on_fail_cb) const
{
    const auto it = m_strmap.find(str.toLower());
    if (it != m_strmap.cend())
        return it->second;

    on_fail_cb();
    return default_val;
}
