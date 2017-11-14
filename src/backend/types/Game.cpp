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


namespace Types {

GameAssets::GameAssets(QObject* parent)
    : QObject(parent)
{}

void GameAssets::setSingle(AssetType key, QString value)
{
    Q_ASSERT(Assets::singleTypes.contains(key));
    m_single_assets[key] = std::move(value);
}

void GameAssets::appendMulti(AssetType key, QString value)
{
    Q_ASSERT(Assets::multiTypes.contains(key));
    m_multi_assets[key].append(value);
}

Game::Game(QString path, QObject* parent)
    : QObject(parent)
    , m_rom_path(std::move(path))
    , m_rom_basename(QFileInfo(m_rom_path).completeBaseName())
    , m_title(m_rom_basename)
    , m_players(1)
    , m_favorite(false)
    , m_rating(0)
    , m_year(0)
    , m_month(0)
    , m_day(0)
    , m_playcount(0)
    , m_assets(new GameAssets(this))
{
    Q_ASSERT(!m_rom_path.isEmpty());
    Q_ASSERT(m_assets);
}

} // namespace Types
