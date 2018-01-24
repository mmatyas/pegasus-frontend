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

namespace Types { class Filters; }


namespace Types {

class Collection : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString tag READ tag CONSTANT)
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(Types::GameList* games READ gameListPtr CONSTANT)

public:
    explicit Collection(QString tag, QObject* parent = nullptr);

    void setName(QString);
    void setCommonLaunchCmd(QString);

    const QString& tag() const { return m_tag; }
    const QString& name() const { return m_name; }
    const QString& launchCmd() const { return m_launch_cmd; }

    const QStringList& sourceDirs() const { return m_source_dirs; }
    QStringList& sourceDirsMut() { return m_source_dirs; }

    const GameList& gameList() const { return m_gamelist; }
    GameList& gameListMut() { return m_gamelist; }

signals:
    void currentGameChanged();
    void launchRequested(const Collection*, const Game*);

private:
    const QString m_tag;
    QString m_name;
    QString m_launch_cmd;
    QStringList m_source_dirs;

    GameList m_gamelist;
    GameList* gameListPtr() { return &m_gamelist; }
};

} // namespace Types
