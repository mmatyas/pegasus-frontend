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

#include "Platform.h"


namespace Types {

class CollectionList : public QObject {
    Q_OBJECT

    Q_PROPERTY(Types::Platform* current
               READ current
               NOTIFY currentChanged)
    Q_PROPERTY(int index
               READ index
               WRITE setIndex
               NOTIFY currentChanged)
    Q_PROPERTY(int count
               READ count
               NOTIFY countChanged)
    Q_PROPERTY(QQmlListProperty<Types::Platform> model
               READ elementsProp
               CONSTANT)

public:
    explicit CollectionList(QObject* parent = nullptr);
    ~CollectionList();

    Platform* current() const;
    int index() const { return m_platform_idx; }
    void setIndex(int);
    int count() const { return m_platforms.count(); }
    QQmlListProperty<Platform> elementsProp();

    const QVector<Platform*>& elements() const { return m_platforms; }
    QVector<Platform*>& elementsMut() { return m_platforms; }

signals:
    void countChanged();
    void currentChanged();
    void currentPlatformGameChanged();

public slots:
    void onScanComplete();

private slots:
    void onPlatformGameChanged();

private:
    QVector<Platform*> m_platforms;
    int m_platform_idx;
};

} // namespace Types
