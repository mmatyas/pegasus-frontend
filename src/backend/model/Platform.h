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

#include "GameList.h"

#include <QString>
#include <QQmlListProperty>

namespace ApiParts { class Filters; }


namespace Model {

class Platform : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString shortName MEMBER m_short_name CONSTANT)
    Q_PROPERTY(QString longName MEMBER m_long_name CONSTANT)
    Q_PROPERTY(Model::GameList* games READ gameListPtr CONSTANT)

public:
    explicit Platform(QString name, QStringList rom_dirs,
                      QStringList rom_filters,
                      QString launch_cmd = QString(),
                      QObject* parent = nullptr);

    const QString m_short_name;
    const QString m_long_name;
    const QStringList m_rom_dirs;
    const QStringList m_rom_filters;
    const QString m_launch_cmd;

    GameList& gameListMut() { return m_gamelist; }
    const GameList& gameList() const { return m_gamelist; }
    GameList* gameListPtr() { return &m_gamelist; }

signals:
    void currentGameChanged();

private:
    GameList m_gamelist;
};

} // namespace Model
