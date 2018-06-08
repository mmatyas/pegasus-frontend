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
#include "modeldata/gaming/Collection.h"

#include <QString>
#include <QQmlListProperty>


namespace model {

class Collection : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QString shortName READ shortName CONSTANT)
    Q_PROPERTY(model::GameList* gameList READ gameListPtr CONSTANT)

public:
    explicit Collection(const modeldata::Collection* const, QObject* parent = nullptr);

    const GameList& gameList() const { return m_gamelist; }
    GameList& gameListMut() { return m_gamelist; }

    const modeldata::Collection& modelData() const { return *m_collection; }

signals:
    void currentGameChanged();
    void gameLaunchRequested(const modeldata::Collection* const, const modeldata::Game* const);
    void gameFavoriteChanged();

private:
    const modeldata::Collection* const m_collection;

    GameList m_gamelist;
    GameList* gameListPtr() { return &m_gamelist; }

    const QString& name() const { return m_collection->name(); }
    const QString& shortName() const { return m_collection->shortName(); }
};

} // namespace model
