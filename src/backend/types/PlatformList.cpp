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


#include "PlatformList.h"

#include "ListPropertyFn.h"

#include <QtConcurrent/QtConcurrent>


namespace Types {

PlatformList::PlatformList(QObject* parent)
    : QObject(parent)
    , m_platform_idx(-1)
    , m_last_scan_duration(0)
{
    connect(&m_datafinder, &DataFinder::platformGamesReady,
            this, &PlatformList::newGamesScanned);
}

PlatformList::~PlatformList()
{
    for (auto& platform : qAsConst(m_platforms))
        delete platform;
}

Platform* PlatformList::current() const
{
    if (m_platform_idx < 0)
        return nullptr;

    Q_ASSERT(m_platform_idx < m_platforms.length());
    return m_platforms.at(m_platform_idx);
}

void PlatformList::setIndex(int idx)
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
    emit currentChanged();
    //emit platformGameChanged();
}

QQmlListProperty<Platform> PlatformList::modelProp()
{
    static const auto count = &listproperty_count<Platform>;
    static const auto at = &listproperty_at<Platform>;

    return {this, &m_platforms, count, at};
}

void PlatformList::startScanning()
{
    // launch the game search on a parallel thread
    QFuture<void> future = QtConcurrent::run([this]{
        QElapsedTimer timer;
        timer.start();

        m_platforms = m_datafinder.find();
        m_last_scan_duration = timer.elapsed();

        // set the correct thread for the QObjects
        for (Platform* const platform : qAsConst(m_platforms)) {
            platform->moveToThread(thread());
            platform->gameListMut().moveToThread(thread());
        }
    });

    m_loading_watcher.setFuture(future);
    connect(&m_loading_watcher, &QFutureWatcher<void>::finished,
            this, &PlatformList::onScanResultsAvailable);
}

void PlatformList::onScanResultsAvailable()
{
    // NOTE: `tr` (see below) uses `int`; assuming we have
    //       less than 2 million games, it will be enough
    int game_count = 0;

    for (int i = 0; i < m_platforms.length(); i++) {
        Platform* const platform_ptr = m_platforms.at(i);
        Q_ASSERT(platform_ptr);

        //connect(platform_ptr, &Platform::currentGameChanged,
        //        [this, i](){ PlatformList::onPlatformGameChanged(i); });
        connect(platform_ptr, &Platform::currentGameChanged,
                this, &PlatformList::currentPlatformGameChanged);

        Platform& platform = *platform_ptr;
        platform.gameListMut().lockGameList();
        game_count += platform.gameListMut().allGames().count();
    }
    qInfo().noquote() << tr("%n games found", "", game_count);

    emit scanCompleted(scanDuration());
    //emit modelChanged();

    if (!m_platforms.isEmpty())
        setIndex(0);
}
/*
void PlatformList::onPlatformGameChanged(int idx)
{
    if (idx == index())
        emit platformGameChanged();
}
*/
/*
void PlatformList::onFiltersChanged(ApiParts::Filters& filters)
{
    // TODO: use QtConcurrent::blockingMap here
    for (Platform* const platform : qAsConst(m_platforms))
        platform->applyFilters(filters);
}
*/
} // namespace Types
