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

Game::Game(modeldata::Game* const game, QObject* parent)
    : QObject(parent)
    , m_game(std::move(game))
    , m_assets(&m_game->assets)
{
    Q_ASSERT(m_game);
}

void Game::setFavorite(bool new_val)
{
    m_game->is_favorite = new_val;
    emit favoriteChanged();
}

void Game::launch()
{
    emit launchRequested(m_game);
}

} // namespace model
