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

#include "PlayniteJsonHelper.h"

#include <QJsonObject>

class QVariant;

namespace providers {
namespace playnite {
JsonObjectHelper::JsonObjectHelper(const QJsonObject& json_object)
    : m_json_map(json_object.toVariantHash())
{}

QString JsonObjectHelper::get_string(const QString& key) const
{
    return m_json_map.value(key).toString();
}

QStringList JsonObjectHelper::get_string_list(const QString& key) const
{
    return m_json_map.value(key).toStringList();
}

QJsonArray JsonObjectHelper::get_json_array(const QString& key) const
{
    return m_json_map.value(key).toJsonArray();
}

JsonObjectHelper JsonObjectHelper::get_json_object_helper(const QString& key) const
{
    return JsonObjectHelper(m_json_map.value(key).toJsonObject());
}

bool JsonObjectHelper::get_bool(const QString& key) const
{
    return m_json_map.value(key, false).toBool();
}

float JsonObjectHelper::get_float(const QString& key) const
{
    return m_json_map.value(key).toFloat();
}

int JsonObjectHelper::get_int(const QString& key) const
{
    return m_json_map.value(key).toInt();
}
} // namespace playnite
} // namespace providers
