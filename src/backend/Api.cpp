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

    connect(&m_collections, &Types::CollectionList::currentChanged,
            this, &ApiObject::currentCollectionChanged);
    connect(&m_collections, &Types::CollectionList::currentGameChanged,
            this, &ApiObject::currentGameChanged);

    connect(&m_datafinder, &DataFinder::totalCountChanged,
            &m_meta, &Types::Meta::onGameCountUpdate);
    connect(&m_datafinder, &DataFinder::metadataSearchStarted,
            &m_meta, &Types::Meta::onScanMetaStarted);

    // partial QML reload
    connect(&m_meta, &Types::Meta::qmlClearCacheRequested,
            this, &ApiObject::qmlClearCacheRequested);
}

void ApiObject::startScanning()
{
    // TODO: reorganize the scanning workflow

    // launch the game search on a parallel thread
    QFuture<void> future = QtConcurrent::run([this]{
        QElapsedTimer timer;
        timer.start();

        m_meta.onScanStarted();
        m_collections.elementsMut() = m_datafinder.find();
        m_meta.onScanCompleted(timer.elapsed());

        // set the correct thread for the QObjects
        for (Types::Collection* const coll : m_collections.elements()) {
            coll->moveToThread(thread());
            coll->gameListMut().moveToThread(thread());
        }
    });

    m_loading_watcher.setFuture(future);

    connect(&m_loading_watcher, &QFutureWatcher<void>::finished,
            &m_collections, &Types::CollectionList::onScanComplete);
    connect(&m_loading_watcher, &QFutureWatcher<void>::finished,
            &m_meta, &Types::Meta::onLoadingCompleted);
}

void ApiObject::launchGame()
{
    if (!currentCollection()) {
        qWarning() << tr("The current collection is undefined, you can't launch any games!");
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
    Q_ASSERT(currentCollection());
    Q_ASSERT(currentGame());
    emit executeLaunch(currentCollection(), currentGame());
}

void ApiObject::onGameFinished()
{
    // TODO: this is where play count could be increased
    emit restoreAfterGame(this);
}

void ApiObject::onFiltersChanged()
{
    for (Types::Collection* const collection : m_collections.elements())
        collection->gameListMut().applyFilters(m_filters);
}
