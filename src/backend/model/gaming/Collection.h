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
#include "modeldata/gaming/CollectionData.h"

#include <QString>
#include <QQmlListProperty>


namespace model {

class Collection : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QString shortName READ shortName CONSTANT)
    Q_PROPERTY(QString summary READ summary CONSTANT)
    Q_PROPERTY(QString description READ description CONSTANT)
    Q_PROPERTY(model::GameAssets* defaultAssets READ defaultAssetsPtr CONSTANT)
    Q_PROPERTY(model::GameList* games READ gameListPtr CONSTANT)

public:
    explicit Collection(modeldata::Collection, QObject* parent = nullptr);

    void setGameList(QVector<Game*>);
    const modeldata::Collection& data() const { return m_collection; }

public:
    const GameList& gameList() const { return m_gamelist; }
    GameList& gameListMut() { return m_gamelist; }

    GameAssets* defaultAssetsPtr() { return &m_default_assets; }

private:
    const QString& name() const { return m_collection.name(); }
    const QString& shortName() const { return m_collection.shortName(); }
    const QString& summary() const { return m_collection.summary; }
    const QString& description() const { return m_collection.description; }
    GameList* gameListPtr() { return &m_gamelist; }

private:
    modeldata::Collection m_collection;
    GameList m_gamelist;
    GameAssets m_default_assets;
};

} // namespace model
