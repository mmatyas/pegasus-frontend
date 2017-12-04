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


#include "Game.h"

#include <QFileInfo>
#include <QDebug>


namespace Types {

Game::Game(QFileInfo fileinfo, QObject* parent)
    : QObject(parent)
    , m_fileinfo(std::move(fileinfo))
    , m_title(m_fileinfo.completeBaseName())
    , m_players(1)
    , m_favorite(false)
    , m_rating(0)
    , m_year(0)
    , m_month(0)
    , m_day(0)
    , m_playcount(0)
{
}

} // namespace Types
