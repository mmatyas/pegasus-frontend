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
    , m_collections(new QQmlObjectListModel<model::Collection>(this))
    , m_launch_game_file(nullptr)
    , m_all_games(new GameListModel(this))
{
    connect(&m_memory, &model::Memory::dataChanged,
            this, &ApiObject::memoryChanged);
}

void ApiObject::clearGameData()
{
    m_collections->clear();

    Q_ASSERT(m_all_games);
    m_all_games->update({});
}

void ApiObject::setGameData(QVector<model::Collection*>&& collections, std::vector<model::Game*>&& games)
{
    Q_ASSERT(m_all_games && m_all_games->entries().empty());
    Q_ASSERT(m_collections && m_collections->isEmpty());

    for (model::Game* const game : qAsConst(games)) {
        game->moveToThread(thread());
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

    for (model::Collection* const coll : qAsConst(collections)) {
        coll->moveToThread(thread());
        coll->setParent(this);
    }

    m_all_games->update(std::move(games));
    m_collections->append(std::move(collections));

    Log::info(LOGMSG("%1 games found").arg(m_all_games->count()));
    emit gamedataReady();
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
    emit gameFileLaunched(m_launch_game_file);
}

void ApiObject::onGameLaunchError(QString msg)
{
    Q_ASSERT(m_launch_game_file);
    m_launch_game_file = nullptr;
    emit eventLaunchError(msg);
}

void ApiObject::onGameProcessFinished()
{
    Q_ASSERT(m_launch_game_file);
    emit gameFileFinished(m_launch_game_file);
    m_launch_game_file = nullptr;
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
