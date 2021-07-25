// Pegasus Frontend
// Copyright (C) 2017-2020  Mátyás Mustoha
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

#include <QJsonArray>

class QVariant;

namespace providers {
namespace playnite {

class JsonObjectHelper {
public:
    explicit JsonObjectHelper(const QJsonObject& json_object);
    QString get_string(const QString& key) const;
    QStringList get_string_list(const QString& key) const;
    QJsonArray get_json_array(const QString& key) const;
    JsonObjectHelper get_json_object_helper(const QString& key) const;
    bool get_bool(const QString& key) const;
    float get_float(const QString& key) const;
    int get_int(const QString& key) const;

private:
    const QHash<QString, QVariant> m_json_map;
};

} // namespace playnite
} // namespace providers
