// Pegasus Frontend
// Copyright (C) 2017-2019  Mátyás Mustoha
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

#include "LocaleUtils.h"


ApiObject::ApiObject(QObject* parent)
    : QObject(parent)
    , m_collections(new QQmlObjectListModel<model::Collection>(this))
    , m_allGames(new QQmlObjectListModel<model::Game>(this))
    , m_launch_game_file(nullptr)
    , m_providerman(this)
{
    connect(&m_memory, &model::Memory::dataChanged,
            this, &ApiObject::memoryChanged);

    connect(&m_internal.settings().locales(), &model::Locales::localeChanged,
            this, &ApiObject::localeChanged);
    connect(&m_internal.settings().keyEditor(), &model::KeyEditor::keysChanged,
            &m_keys, &model::Keys::refresh_keys);
    connect(&m_internal.settings().themes(), &model::Themes::themeChanged,
            this, &ApiObject::onThemeChanged);
    connect(&m_internal.settings(), &model::Settings::providerReloadingRequested,
            this, &ApiObject::startScanning);

    connect(&m_providerman, &ProviderManager::gameCountChanged,
            &m_internal.meta(), &model::Meta::onGameCountUpdate);
    connect(&m_providerman, &ProviderManager::firstPhaseComplete,
            &m_internal.meta(), &model::Meta::onFirstPhaseCompleted);
    connect(&m_providerman, &ProviderManager::secondPhaseComplete,
            &m_internal.meta(), &model::Meta::onSecondPhaseCompleted);
    connect(&m_providerman, &ProviderManager::staticDataReady,
            this, &ApiObject::onStaticDataLoaded);

    onThemeChanged();
}

void ApiObject::startScanning()
{
    m_internal.meta().startLoading();
    emit eventLoadingStarted();

    m_collections->clear();
    m_allGames->clear();

    m_providerman.startStaticSearch(m_providerman_collections, m_providerman_games);
}

void ApiObject::onStaticDataLoaded()
{
    for (model::Game* const game : m_providerman_games) {
        Q_ASSERT(game->parent() == nullptr);
        game->setParent(this);

        connect(game, &model::Game::launchFileSelectorRequested,
                this, &ApiObject::onGameFileSelectorRequested);
        connect(game, &model::Game::favoriteChanged,
                this, &ApiObject::onGameFavoriteChanged);

        for (model::GameFile* const gamefile : game->filesConst()) {
            connect(gamefile, &model::GameFile::launchRequested,
                    this, &ApiObject::onGameFileLaunchRequested);
        }
    }
    for (model::Collection* const coll : m_providerman_collections) {
        Q_ASSERT(coll->parent() == nullptr);
        coll->setParent(this);
    }

    QVector<model::Game*> game_vec;
    std::swap(m_providerman_games, game_vec);
    m_allGames->append(std::move(game_vec));

    QVector<model::Collection*> coll_vec;
    std::swap(m_providerman_collections, coll_vec);
    m_collections->append(std::move(coll_vec));

    m_internal.meta().onUiReady();
    qInfo().noquote() << tr_log("%1 games found").arg(m_allGames->count());

    m_providerman.startDynamicSearch(m_allGames->asList(), m_collections->asList());
}

void ApiObject::onGameFileSelectorRequested()
{
    auto game = static_cast<model::Game*>(QObject::sender());
    emit eventSelectGameFile(game);
}

void ApiObject::onGameFileLaunchRequested()
{
    if (m_launch_game_file)
        return;

    m_launch_game_file = static_cast<model::GameFile*>(QObject::sender());
    emit launchGameFile(m_launch_game_file);
}

void ApiObject::onGameLaunchOk()
{
    Q_ASSERT(m_launch_game_file);
    m_providerman.onGameLaunched(m_launch_game_file);
}

void ApiObject::onGameLaunchError(QString msg)
{
    Q_ASSERT(m_launch_game_file);
    m_launch_game_file = nullptr;

    emit eventLaunchError(msg);
}

void ApiObject::onGameFinished()
{
    Q_ASSERT(m_launch_game_file);

    m_providerman.onGameFinished(m_launch_game_file);
    m_launch_game_file = nullptr;
}

void ApiObject::onGameFavoriteChanged()
{
    m_providerman.onGameFavoriteChanged(m_allGames->asList());
}

void ApiObject::onThemeChanged()
{
    m_memory.changeTheme(m_internal.settings().themes().currentQmlDir());
}
