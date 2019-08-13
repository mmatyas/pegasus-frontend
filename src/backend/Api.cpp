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


namespace {
void sort_q_games(QVector<model::Game*>& games)
{
    std::sort(games.begin(), games.end(),
        [](const model::Game* const a, const model::Game* const b) {
            return QString::localeAwareCompare(a->title(), b->title()) < 0;
        }
    );
}

void sort_q_collections(QVector<model::Collection*>& collections)
{
    std::sort(collections.begin(), collections.end(),
        [](const model::Collection* const a, const model::Collection* const b) {
            return QString::localeAwareCompare(a->name(), b->name()) < 0;
        }
    );
}

void build_ui_layer(providers::SearchContext& sctx,
                    QQmlObjectListModel<model::Game>& game_model,
                    QQmlObjectListModel<model::Collection>& collection_model)
{
    HashMap<size_t, model::Game*> q_game_map;
    q_game_map.reserve(sctx.games.size());

    for (auto& keyval : sctx.games) {
        auto q_game = new model::Game(std::move(keyval.second));
        q_game_map.emplace(keyval.first, q_game);
    }

    QVector<model::Game*> q_game_list;
    q_game_list.reserve(static_cast<int>(sctx.games.size()));
    for (auto& keyval : q_game_map)
        q_game_list.append(keyval.second);

    sort_q_games(q_game_list);
    game_model.append(q_game_list);


    QVector<model::Collection*> q_collections;
    q_collections.reserve(static_cast<int>(sctx.collections.size()));

    for (auto& keyval : sctx.collections) {
        auto q_coll = new model::Collection(std::move(keyval.second));
        q_collections.append(q_coll);
    }

    sort_q_collections(q_collections);
    collection_model.append(q_collections);


    for (model::Collection* const q_coll : q_collections) {
        Q_ASSERT(sctx.collection_childs.count(q_coll->name()));
        const std::vector<size_t>& game_ids = sctx.collection_childs[q_coll->name()];

        QVector<model::Game*> q_childs;
        q_childs.reserve(static_cast<int>(game_ids.size()));

        for (size_t game_id : game_ids) {
            Q_ASSERT(q_game_map.count(game_id));
            q_childs.append(q_game_map.at(game_id));
        }

        sort_q_games(q_childs);
        q_coll->setGameList(q_childs);
    }
}
} // namespace


ApiObject::ApiObject(QObject* parent)
    : QObject(parent)
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

    m_collections.clear();
    m_allGames.clear();

    m_providerman.startStaticSearch(m_providerman_sctx);
}

void ApiObject::onStaticDataLoaded()
{
    build_ui_layer(m_providerman_sctx, m_allGames, m_collections);

    for (model::Game* const game : qAsConst(m_allGames)) {
        connect(game, &model::Game::launchFileSelectorRequested,
                this, &ApiObject::onGameFileSelectorRequested);
        connect(game, &model::Game::favoriteChanged,
                this, &ApiObject::onGameFavoriteChanged);

        for (model::GameFile* const gamefile : *game->files()) {
            connect(gamefile, &model::GameFile::launchRequested,
                    this, &ApiObject::onGameFileLaunchRequested);
        }
    }

    m_internal.meta().onUiReady();
    qInfo().noquote() << tr_log("%1 games found").arg(m_allGames.count());

    m_providerman_sctx = providers::SearchContext();
    m_providerman.startDynamicSearch(m_allGames.asList(), m_collections.asList());
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
    m_providerman.onGameFavoriteChanged(m_allGames.asList());
}

void ApiObject::onThemeChanged()
{
    m_memory.changeTheme(m_internal.settings().themes().currentQmlDir());
}
