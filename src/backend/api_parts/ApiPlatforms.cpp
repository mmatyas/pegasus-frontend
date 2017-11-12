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


#include "ApiPlatforms.h"

#include "ListPropertyFn.h"

#include <QtConcurrent/QtConcurrent>


namespace ApiParts {

Platforms::Platforms(QObject* parent)
    : QObject(parent)
    , m_platform_idx(-1)
    , m_last_scan_duration(0)
{
    connect(&m_datafinder, &DataFinder::platformGamesReady,
            this, &ApiParts::Platforms::newGamesScanned);
}

Platforms::~Platforms()
{
    for (auto& platform : qAsConst(m_platforms))
        delete platform;
}

Model::Platform* Platforms::currentPlatform() const
{
    if (m_platform_idx < 0)
        return nullptr;

    Q_ASSERT(m_platform_idx < m_platforms.length());
    return m_platforms.at(m_platform_idx);
}

void Platforms::setCurrentIndex(int idx)
{
    // Setting the index to a valid value causes changing the current platform
    // and the current game. Setting the index to an invalid value should not
    // change anything.

    if (idx == m_platform_idx)
        return;

    const bool valid_idx = (0 <= idx || idx < m_platforms.count());
    if (!valid_idx) {
        qWarning() << tr("Invalid platform index #%1").arg(idx);
        return;
    }

    m_platform_idx = idx;
    emit platformChanged();
    emit platformGameChanged();
}

QQmlListProperty<Model::Platform> Platforms::getListProp()
{
    static const auto count = &listproperty_count<Model::Platform>;
    static const auto at = &listproperty_at<Model::Platform>;

    return {this, &m_platforms, count, at};
}

void Platforms::startScanning()
{
    // launch the game search on a parallel thread
    QFuture<void> future = QtConcurrent::run([this]{
        QElapsedTimer timer;
        timer.start();

        m_platforms = m_datafinder.find();
        m_last_scan_duration = timer.elapsed();

        // set the correct thread for the QObjects
        for (Model::Platform* const platform : qAsConst(m_platforms))
            platform->moveToThread(thread());
    });

    m_loading_watcher.setFuture(future);
    connect(&m_loading_watcher, &QFutureWatcher<void>::finished,
            this, &ApiParts::Platforms::onScanResultsAvailable);
}

void Platforms::onScanResultsAvailable()
{
    // NOTE: `tr` (see below) uses `int`; assuming we have
    //       less than 2 million games, it will be enough
    int game_count = 0;

    for (int i = 0; i < m_platforms.length(); i++) {
        Model::Platform* const platform_ptr = m_platforms.at(i);
        Q_ASSERT(platform_ptr);

        connect(platform_ptr, &Model::Platform::currentGameChanged,
                [this, i](){ ApiParts::Platforms::onPlatformGameChanged(i); });

        Model::Platform& platform = *platform_ptr;
        platform.lockGameList();
        game_count += platform.allGames().count();
    }
    qInfo().noquote() << tr("%n games found", "", game_count);

    emit scanCompleted(scanDuration());
    emit modelChanged();

    if (!m_platforms.isEmpty())
        setCurrentIndex(0);
}

void Platforms::onPlatformGameChanged(int idx)
{
    if (idx == currentIndex())
        emit platformGameChanged();
}

void Platforms::onFiltersChanged(ApiParts::Filters& filters)
{
    // TODO: use QtConcurrent::blockingMap here
    for (Model::Platform* const platform : qAsConst(m_platforms))
        platform->applyFilters(filters);
}

} // namespace ApiParts
