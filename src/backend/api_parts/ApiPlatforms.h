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
#include "model/Platform.h"

#include <QObject>
#include <QFutureWatcher>


namespace ApiParts {

class Platforms : public QObject {
    Q_OBJECT

public:
    explicit Platforms(QObject* parent = nullptr);
    ~Platforms();

    int currentIndex() const { return m_platform_idx; }
    void setCurrentIndex(int);
    Model::Platform* currentPlatform() const;
    QQmlListProperty<Model::Platform> getListProp();

    void startScanning();
    qint64 scanDuration() const { return m_last_scan_duration; }

signals:
    void modelChanged();
    void platformChanged();
    void platformGameChanged();

    void newGamesScanned(int game_count);
    void scanCompleted(qint64 elapsedTime);

public slots:
    void onFiltersChanged(ApiParts::Filters&);

private:
    QVector<Model::Platform*> m_platforms;
    int m_platform_idx;

    // initialization
    DataFinder m_datafinder;
    QFutureWatcher<void> m_loading_watcher;
    qint64 m_last_scan_duration;

    void onScanResultsAvailable();
    void onPlatformGameChanged(int platformIndex);
};

} // namespace ApiParts
