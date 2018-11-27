// Pegasus Frontend
// Copyright (C) 2017-2018  Mátyás Mustoha
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
    , m_providerman(this)
{
    connect(m_internal.settings.localesPtr(), &model::LocaleList::localeChanged,
            this, &ApiObject::localeChanged);
    connect(m_internal.settings.keyEditorPtr(), &model::KeyEditor::keysChanged,
            &m_keys, &model::Keys::refresh_keys);
    connect(&m_internal.system, &model::System::appCloseRequested,
            this, &ApiObject::appCloseRequested);
    connect(&m_providerman, &ProviderManager::gameCountChanged,
            &m_internal.meta, &model::Meta::onGameCountUpdate);
    connect(&m_providerman, &ProviderManager::firstPhaseComplete,
            &m_internal.meta, &model::Meta::onFirstPhaseCompleted);
    connect(&m_providerman, &ProviderManager::secondPhaseComplete,
            &m_internal.meta, &model::Meta::onSecondPhaseCompleted);

    connect(&m_filters, &model::Filters::filtersChanged,
            this, &ApiObject::onFiltersChanged);

    connect(&m_providerman, &ProviderManager::staticDataReady,
            this, &ApiObject::onStaticDataLoaded);

    connect(&m_collections, &model::CollectionList::currentChanged,
            this, &ApiObject::currentCollectionChanged);
    connect(&m_collections, &model::CollectionList::currentGameChanged,
            this, &ApiObject::currentGameChanged);
    connect(&m_collections, &model::CollectionList::gameLaunchRequested,
            this, &ApiObject::onLaunchRequested);
    connect(&m_collections, &model::CollectionList::gameFavoriteChanged,
            this, &ApiObject::onGameFavoriteChanged);

    // partial QML reload
    connect(&m_internal.meta, &model::Meta::qmlClearCacheRequested,
            this, &ApiObject::qmlClearCacheRequested);
}

void ApiObject::startScanning()
{
    m_providerman.startSearch();
}

void ApiObject::onStaticDataLoaded(QVector<model::Collection*> collections, QVector<model::Game*> games)
{
    m_collections.setModelData(std::move(collections), std::move(games));
    m_internal.meta.onUiReady();
}

void ApiObject::onLaunchRequested(model::Collection* coll, model::Game* game)
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

    m_providerman.onGameLaunched(m_launch_collection, m_launch_game);
}

void ApiObject::onGameLaunchError()
{
    Q_ASSERT(m_launch_collection);
    Q_ASSERT(m_launch_game);

    // TODO: show error
    m_launch_collection = nullptr;
    m_launch_game = nullptr;
}

void ApiObject::onGameFinished()
{
    Q_ASSERT(m_launch_collection);
    Q_ASSERT(m_launch_game);

    m_providerman.onGameFinished(m_launch_collection, m_launch_game);

    m_launch_collection = nullptr;
    m_launch_game = nullptr;
}

void ApiObject::onGameFavoriteChanged()
{
    m_providerman.onGameFavoriteChanged(m_collections.allGames());
}

void ApiObject::onFiltersChanged()
{
    for (model::Collection* const collection : m_collections.collections())
        collection->gameListMut().applyFilters(m_filters);
}
