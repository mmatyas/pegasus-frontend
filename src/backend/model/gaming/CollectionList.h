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

#include "Collection.h"

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
    ~CollectionList();

    void setModelData(std::vector<modeldata::Collection>);
    const std::vector<modeldata::Collection>& modelData() const { return m_modeldata; }

    Collection* current() const;
    int count() const { return m_collections.count(); }

    int index() const { return m_collection_idx; }
    void setIndex(int);
    Q_INVOKABLE void incrementIndex();
    Q_INVOKABLE void decrementIndex();
    Q_INVOKABLE void incrementIndexNoWrap();
    Q_INVOKABLE void decrementIndexNoWrap();

    QQmlListProperty<Collection> elementsProp();
    const QVector<Collection*>& elements() const { return m_collections; }

signals:
    void modelChanged();
    void currentChanged();
    void currentGameChanged();
    void gameLaunchRequested(const modeldata::Collection* const, const modeldata::Game* const);
    void gameFavoriteChanged();

private slots:
    void onGameChanged();

private:
    std::vector<modeldata::Collection> m_modeldata;

    QVector<Collection*> m_collections;
    int m_collection_idx;

    void shiftIndex(IndexShiftDirection);
};

} // namespace model
