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

#include "Log.h"
#include "model/gaming/GameFile.h"


namespace model {
ApiObject::ApiObject(const backend::CliArgs&, QObject* parent)
    : QObject(parent)
    , m_launch_game(nullptr)
    , m_collections(new CollectionListModel(this))
    , m_all_games(new GameListModel(this))
{
    connect(&m_memory, &model::Memory::dataChanged,
            this, &ApiObject::memoryChanged);
}

void ApiObject::clearGameData()
{
    Q_ASSERT(m_collections);
    m_collections->update({});

    Q_ASSERT(m_all_games);
    m_all_games->update({});
}

void ApiObject::setGameData(std::vector<model::Collection*>&& collections, std::vector<model::Game*>&& games)
{
    Q_ASSERT(m_all_games && m_all_games->entries().empty());
    Q_ASSERT(m_collections && m_collections->entries().empty());

    for (model::Game* const game : qAsConst(games)) {
        game->moveToThread(thread());
        game->setParent(this);

        connect(game, &model::Game::launchRequested,
                this, &ApiObject::onGameLaunchRequested);
        connect(game, &model::Game::launchFileSelectorRequested,
                this, &ApiObject::onGameFileSelectorRequested);
        connect(game, &model::Game::favoriteChanged,
                this, &ApiObject::onGameFavoriteChanged);

        for (model::GameFile* const gamefile : game->filesModel()->entries()) {
            connect(gamefile, &model::GameFile::launchRequested,
                    this, &ApiObject::onGameFileLaunchRequested);
        }
    }

    for (model::Collection* const coll : qAsConst(collections)) {
        coll->moveToThread(thread());
        coll->setParent(this);
    }

    m_all_games->update(std::move(games));
    m_collections->update(std::move(collections));

    Log::info(LOGMSG("%1 games found").arg(m_all_games->count()));
    emit gamedataReady();
}

void ApiObject::onGameFileSelectorRequested()
{
    auto game = static_cast<model::Game*>(QObject::sender());
    emit eventSelectGameFile(game);
}

void ApiObject::onGameLaunchRequested()
{
    if (m_launch_game)
        return;

    model::Game* game = static_cast<model::Game*>(QObject::sender());
    m_launch_game = new model::GameLaunchPair(game, nullptr);
    emit launchGame(m_launch_game);
}
void ApiObject::onGameFileLaunchRequested()
{
    if (m_launch_game)
        return;

    model::GameFile* gamefile = static_cast<model::GameFile*>(QObject::sender());
    m_launch_game = new model::GameLaunchPair(gamefile->parentGame(), gamefile);
    emit launchGame(m_launch_game);
}

void ApiObject::onGameLaunchOk()
{
    Q_ASSERT(m_launch_game);
    emit gameLaunched(m_launch_game);
}

void ApiObject::onGameLaunchError(QString msg)
{
    Q_ASSERT(m_launch_game);
    m_launch_game = nullptr;
    emit eventLaunchError(msg);
}

void ApiObject::onGameProcessFinished()
{
    Q_ASSERT(m_launch_game);
    emit gameFinished(m_launch_game);
    m_launch_game = nullptr;
}

void ApiObject::onGameFavoriteChanged()
{
    emit favoritesChanged();
}

void ApiObject::onLocaleChanged()
{
    emit retranslationRequested();
}

void ApiObject::onThemeChanged(QString theme_dir)
{
    m_memory.changeTheme(theme_dir);
}
} // namespace model
