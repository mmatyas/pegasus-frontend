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

#include "DataFinder.h"
#include "Platform.h"

#include <QObject>
#include <QFutureWatcher>


namespace Types {

class PlatformList : public QObject {
    Q_OBJECT

    Q_PROPERTY(Types::Platform* current
               READ current
               NOTIFY currentChanged)
    Q_PROPERTY(int index
               READ index
               WRITE setIndex
               NOTIFY currentChanged)
    Q_PROPERTY(int count
               READ count CONSTANT)
    Q_PROPERTY(QQmlListProperty<Types::Platform> model
               READ modelProp
               CONSTANT)

public:
    explicit PlatformList(QObject* parent = nullptr);
    ~PlatformList();

    int index() const { return m_platform_idx; }
    void setIndex(int);
    Platform* current() const;

    int count() const { return m_platforms.count(); }
    QQmlListProperty<Platform> modelProp();
    const QVector<Platform*>& model() const { return m_platforms; }

    void startScanning();
    qint64 scanDuration() const { return m_last_scan_duration; }

signals:
    //void modelChanged();
    void currentChanged();
    void currentPlatformGameChanged();

    void newGamesScanned(int game_count);
    void scanCompleted(qint64 elapsedTime);

private:
    QVector<Platform*> m_platforms;
    int m_platform_idx;

    // initialization
    DataFinder m_datafinder;
    QFutureWatcher<void> m_loading_watcher;
    qint64 m_last_scan_duration;

    void onScanResultsAvailable();
    void onPlatformGameChanged(int platformIndex);
};

} // namespace Types
