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

#include <QObject>
#include <QString>


namespace Types {

/// An utility class to contain theme informations
class Theme : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString qmlPath MEMBER m_root_qml CONSTANT)

    Q_PROPERTY(QString name MEMBER m_name CONSTANT)
    Q_PROPERTY(QString author MEMBER m_author CONSTANT)
    Q_PROPERTY(QString version MEMBER m_version CONSTANT)
    Q_PROPERTY(QString summary MEMBER m_summary CONSTANT)
    Q_PROPERTY(QString description MEMBER m_description CONSTANT)

public:
    // this ctor expects many parameters to make all class fields const
    // and avoid implementing and sending change signals and setters
    explicit Theme(QString root_dir, QString root_qml, QString name,
                   QString author = QString(),
                   QString version = QString(),
                   QString summary = QString(),
                   QString description = QString(),
                   QObject* parent = nullptr);

    const QString& dir() const { return m_root_dir; }
    const QString& name() const { return m_name; }

    int compare(const Theme& other) const;

private:
    const QString m_root_dir;
    const QString m_root_qml;

    const QString m_name;
    const QString m_author;
    const QString m_version;
    const QString m_summary;
    const QString m_description;
};

} // namespace Types
