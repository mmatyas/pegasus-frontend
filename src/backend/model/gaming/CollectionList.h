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

    void setModelData(const QVector<Collection*>&);

    Collection* current() const;
    const QVector<Collection*>& collections() const { return m_collections; }

signals:
    void modelChanged();
    void currentChanged();

private:
    int count() const { return m_collections.count(); }
    int index() const { return m_collection_idx; }
    void setIndex(int);

    QQmlListProperty<Collection> elementsProp();

private:
    QVector<Collection*> m_collections;
    int m_collection_idx;
};

} // namespace model
