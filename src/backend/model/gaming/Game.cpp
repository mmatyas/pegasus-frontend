// Pegasus Frontend
// Copyright (C) 2018  Mátyás Mustoha
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
    , m_game(std::move(game))
    , m_assets(&m_game.assets, this)
{
}

void Game::setFavorite(bool new_val)
{
    m_game.is_favorite = new_val;
    emit favoriteChanged();
}

// This one is for summing the play times provided by multiple Providers.
void Game::addPlayStats(int playcount, qint64 playtime, const QDateTime& last_played)
{
    m_game.last_played = std::max(m_game.last_played, last_played);
    m_game.playtime += playtime;
    m_game.playcount += playcount;
    emit playStatsChanged();
}

// This one is a single update for playtime when the game finishes.
void Game::updatePlayStats(qint64 duration, QDateTime time_finished)
{
    m_game.last_played = std::move(time_finished);
    m_game.playtime += duration;
    m_game.playcount++;
    emit playStatsChanged();
}

void Game::launch()
{
    emit launchRequested(this);
}

} // namespace model
