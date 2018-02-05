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


#include "Assets.h"


const QVector<AssetType> Assets::singleTypes = {
    AssetType::BOX_FRONT,
    AssetType::BOX_BACK,
    AssetType::BOX_SPINE,
    AssetType::BOX_FULL,
    AssetType::CARTRIDGE,
    AssetType::LOGO,
    AssetType::POSTER,
    AssetType::ARCADE_MARQUEE,
    AssetType::ARCADE_BEZEL,
    AssetType::ARCADE_PANEL,
    AssetType::ARCADE_CABINET_L,
    AssetType::ARCADE_CABINET_R,
    AssetType::UI_TILE,
    AssetType::UI_BANNER,
    AssetType::UI_STEAMGRID,
    AssetType::BACKGROUND,
    AssetType::MUSIC,
};
const QVector<AssetType> Assets::multiTypes = {
    AssetType::SCREENSHOTS,
    AssetType::VIDEOS,
};
