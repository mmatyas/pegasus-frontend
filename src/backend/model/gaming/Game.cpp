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


#include "Game.h"


namespace model {

Game::Game(modeldata::Game game, QObject* parent)
    : QObject(parent)
    , m_files(this)
    , m_game(std::move(game))
    , m_assets(&m_game.assets, this)
{
    for (auto& entry : m_game.files) {
        auto gamefile = new model::GameFile(std::move(entry.second), this);

        connect(gamefile, &model::GameFile::playStatsChanged,
                this, &model::Game::playStatsChanged);

        m_files.append(gamefile);
    }

    m_game.files.clear();
}

void Game::setFavorite(bool new_val)
{
    m_game.is_favorite = new_val;
    emit favoriteChanged();
}


int Game::playCount() const
{
    return std::accumulate(filesConst().cbegin(), filesConst().cend(), 0,
        [](int sum, const model::GameFile* const gamefile){
            return sum + gamefile->playCount();
        });
}
qint64 Game::playTime() const
{
    return std::accumulate(filesConst().cbegin(), filesConst().cend(), 0,
        [](qint64 sum, const model::GameFile* const gamefile){
            return sum + gamefile->playTime();
        });
}
QDateTime Game::lastPlayed() const
{
    const auto it = std::max_element(filesConst().cbegin(), filesConst().cend(),
        [](const model::GameFile* const a, const model::GameFile* const b){
            return a->lastPlayed() < b->lastPlayed();
        });
    return (*it)->lastPlayed();
}

void Game::launch()
{
    Q_ASSERT(m_files.count() > 0);

    if (m_files.count() == 1)
        m_files.first()->launch();
    else
        emit launchFileSelectorRequested();
}

} // namespace model
