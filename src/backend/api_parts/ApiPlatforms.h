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

#include "model/Platform.h"

#include <QObject>
#include <QFutureWatcher>


namespace ApiParts {

class Platforms : public QObject {
    Q_OBJECT

public:
    explicit Platforms(QObject* parent = nullptr);
    ~Platforms();

    int currentIndex() const { return m_current_platform_idx; }
    Model::Platform* currentPlatform() const { return m_current_platform; }

    void setIndex(int);
    void resetIndex();

    QList<Model::Platform*>& allPlatforms() { return m_platforms; }

    void startScanning();
    qint64 scanDuration() const { return m_last_scan_duration; }

signals:
    void modelChanged();
    void indexChanged();
    void platformChanged();
    void platformGameChanged();
    void scanCompleted(qint64 elapsedTime);

public slots:
    void onFiltersChanged(ApiParts::Filters&);

private:
    QList<Model::Platform*> m_platforms;

    int m_current_platform_idx;
    Model::Platform* m_current_platform;

    // initialization
    QFutureWatcher<void> m_loading_watcher;
    qint64 m_last_scan_duration;

    void onScanResultsAvailable();
    void onPlatformGameChanged(int platformIndex);
};

} // namespace ApiParts
