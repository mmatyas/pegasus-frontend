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


#include "Api.h"

#include <QtConcurrent/QtConcurrent>


ApiObject::ApiObject(QObject* parent)
    : QObject(parent)
{
    connect(m_settings.localesPtr(), &Types::LocaleList::localeChanged,
            this, &ApiObject::localeChanged);
    connect(&m_system, &Types::System::appCloseRequested,
            this, &ApiObject::appCloseRequested);
    connect(&m_filters, &Types::Filters::filtersChanged,
            this, &ApiObject::onFiltersChanged);

    connect(&m_platform_list, &Types::PlatformList::currentChanged,
            this, &ApiObject::currentPlatformChanged);
    connect(&m_platform_list, &Types::PlatformList::currentPlatformGameChanged,
            this, &ApiObject::currentGameChanged);

    connect(&m_datafinder, &DataFinder::platformGamesReady,
            &m_meta, &Types::Meta::onNewGamesScanned);
}

void ApiObject::startScanning()
{
    // TODO: reorganize the scanning workflow

    // launch the game search on a parallel thread
    QFuture<void> future = QtConcurrent::run([this]{
        QElapsedTimer timer;
        timer.start();

        m_meta.onScanStarted();
        m_platform_list.platformsMut() = m_datafinder.find();
        m_meta.onScanCompleted(timer.elapsed());

        // set the correct thread for the QObjects
        for (Types::Platform* const platform : m_platform_list.platforms()) {
            platform->moveToThread(thread());
            platform->gameListMut().moveToThread(thread());
        }
    });

    m_loading_watcher.setFuture(future);

    connect(&m_loading_watcher, &QFutureWatcher<void>::finished,
            &m_platform_list, &Types::PlatformList::onScanComplete);
    connect(&m_loading_watcher, &QFutureWatcher<void>::finished,
            &m_meta, &Types::Meta::onLoadingCompleted);
}

void ApiObject::launchGame()
{
    if (!currentPlatform()) {
        qWarning() << tr("The current platform is undefined, you can't launch any games!");
        return;
    }
    if (!currentGame()) {
        qWarning() << tr("The current game is undefined, you can't launch it!");
        return;
    }

    emit prepareLaunch();
}

void ApiObject::onReadyToLaunch()
{
    Q_ASSERT(currentPlatform());
    Q_ASSERT(currentGame());
    emit executeLaunch(currentPlatform(), currentGame());
}

void ApiObject::onGameFinished()
{
    // TODO: this is where play count could be increased
    emit restoreAfterGame(this);
}

void ApiObject::onFiltersChanged()
{
    for (Types::Platform* const platform : m_platform_list.platforms())
        platform->gameListMut().applyFilters(m_filters);
}
