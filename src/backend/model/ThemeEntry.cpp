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


#include "ThemeEntry.h"


namespace Model {

Theme::Theme(QString root_dir, QString root_qml, QString name,
             QString author, QString version, QString summary, QString description,
             QObject* parent)
    : QObject(parent)
    , m_root_dir(root_dir)
    , m_root_qml(root_qml.startsWith(":") ? "qrc" + root_qml : "file:" + root_qml)
    , m_name(name)
    , m_author(author)
    , m_version(version)
    , m_summary(summary)
    , m_description(description)
{}

int Theme::compare(const Theme& other) const
{
    return QString::localeAwareCompare(m_name, other.m_name);
}

} // namespace Model
