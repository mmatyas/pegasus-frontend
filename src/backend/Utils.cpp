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


#include "Utils.h"

#include "utils/HashMap.h"

#include <QFileInfo>

#ifdef Q_OS_UNIX
#include <sys/stat.h>
#endif


namespace {
const HashMap<QString, const bool> str_to_bool_map {
    { QStringLiteral("yes"), true },
    { QStringLiteral("on"), true },
    { QStringLiteral("true"), true },
    { QStringLiteral("no"), false },
    { QStringLiteral("off"), false },
    { QStringLiteral("false"), false },
};
} // namespace


bool validExtPath(const QString& path) {
#ifdef Q_OS_UNIX
    // fast posix check for unix systems
    static struct ::stat buffer;
    return (::stat(path.toUtf8().constData(), &buffer) == 0);
#else
    // default Qt fallback
    return QFileInfo::exists(path);
#endif
}

bool validFile(const QString& path)
{
    QFileInfo file(path);
    return file.exists() && file.isFile();
}

bool is_str_bool(const QString& str)
{
    return str_to_bool_map.count(str.toLower());
}

bool str_to_bool(const QString& str, const bool default_val, const std::function<void()>& on_fail_cb)
{
    const auto it = str_to_bool_map.find(str.toLower());
    if (it != str_to_bool_map.cend())
        return it->second;

    on_fail_cb();
    return default_val;
}

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
