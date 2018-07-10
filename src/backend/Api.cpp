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
    , m_launch_collection(nullptr)
    , m_launch_game(nullptr)
{
    connect(m_settings.localesPtr(), &model::LocaleList::localeChanged,
            this, &ApiObject::localeChanged);
    connect(&m_system, &model::System::appCloseRequested,
            this, &ApiObject::appCloseRequested);
    connect(&m_filters, &model::Filters::filtersChanged,
            this, &ApiObject::onFiltersChanged);

    connect(&m_collections, &model::CollectionList::currentChanged,
            this, &ApiObject::currentCollectionChanged);
    connect(&m_collections, &model::CollectionList::currentGameChanged,
            this, &ApiObject::currentGameChanged);
    connect(&m_collections, &model::CollectionList::gameLaunchRequested,
            this, &ApiObject::onLaunchRequested);
    connect(&m_collections, &model::CollectionList::gameFavoriteChanged,
            this, &ApiObject::onGameFavoriteChanged);

    connect(&m_datafinder, &DataFinder::totalCountChanged,
            &m_meta, &model::Meta::onGameCountUpdate);
    connect(&m_datafinder, &DataFinder::metadataSearchStarted,
            &m_meta, &model::Meta::onScanMetaStarted);

    // partial QML reload
    connect(&m_meta, &model::Meta::qmlClearCacheRequested,
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
        m_gaming_data = m_datafinder.find();
        m_meta.onScanCompleted(timer.elapsed());
    });
    m_loading_watcher.setFuture(future);

    connect(&m_loading_watcher, &QFutureWatcher<void>::finished,
            this, &ApiObject::onScanComplete);
}

void ApiObject::onScanComplete()
{
    m_collections.setModelData(m_gaming_data);
    m_meta.onLoadingCompleted();
}

void ApiObject::onLaunchRequested(const modeldata::Collection* const coll, const modeldata::Game* const game)
{
    // avoid launch spamming
    if (m_launch_game)
        return;

    m_launch_collection = coll;
    m_launch_game = game;

    emit launchGame(m_launch_collection, m_launch_game);
}

void ApiObject::onGameLaunchOk()
{
    Q_ASSERT(m_launch_game);

    // TODO: start play timer here and increase play count
}

void ApiObject::onGameLaunchError()
{
    Q_ASSERT(m_launch_game);

    // TODO: show error
    onGameFinished();
}

void ApiObject::onGameFinished()
{
    m_launch_collection = nullptr;
    m_launch_game = nullptr;
}

void ApiObject::onGameFavoriteChanged()
{
    m_favorite_writer.queueTask(m_gaming_data);
}

void ApiObject::onFiltersChanged()
{
    for (model::Collection* const collection : m_collections.elements())
        collection->gameListMut().applyFilters(m_filters);
}
