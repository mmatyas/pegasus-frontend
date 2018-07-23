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

#include "model/gaming/Collection.h"

#include <QObject>
#include <QQmlListProperty>
#include <QVector>

enum class IndexShiftDirection : unsigned char;


namespace model {

class CollectionList : public QObject {
    Q_OBJECT

    Q_PROPERTY(model::Collection* current
               READ current
               NOTIFY currentChanged)
    Q_PROPERTY(int index
               READ index
               WRITE setIndex
               NOTIFY currentChanged)
    Q_PROPERTY(int count
               READ count
               NOTIFY modelChanged)
    Q_PROPERTY(QQmlListProperty<model::Collection> model
               READ elementsProp
               NOTIFY modelChanged)

public:
    explicit CollectionList(QObject* parent = nullptr);

    void setModelData(QVector<Collection*>&&, QVector<Game*>&&);

    Collection* current() const;
    const QVector<Collection*>& collections() const { return m_collections; }
    const QVector<Game*>& allGames() const { return m_all_games; }

    Q_INVOKABLE void incrementIndex();
    Q_INVOKABLE void decrementIndex();
    Q_INVOKABLE void incrementIndexNoWrap();
    Q_INVOKABLE void decrementIndexNoWrap();

signals:
    void modelChanged();
    void currentChanged();
    void currentGameChanged();
    void gameLaunchRequested(const model::Collection* const, const model::Game* const);
    void gameFavoriteChanged();

private slots:
    void onGameChanged();

private:
    int count() const { return m_collections.count(); }
    int index() const { return m_collection_idx; }
    void setIndex(int);

    QQmlListProperty<Collection> elementsProp();

private:
    QVector<Collection*> m_collections;
    int m_collection_idx;

    void shiftIndex(IndexShiftDirection);

    QVector<Game*> m_all_games;
};

} // namespace model
