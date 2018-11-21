// Pegasus Frontend
// Copyright (C) 2017-2018  Mátyás Mustoha
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


#pragma once

enum class AssetType : unsigned char {
    UNKNOWN,

    BOX_FRONT,
    BOX_BACK,
    BOX_SPINE,
    BOX_FULL,
    CARTRIDGE,
    LOGO,
    POSTER,

    ARCADE_MARQUEE,
    ARCADE_BEZEL,
    ARCADE_PANEL,
    ARCADE_CABINET_L,
    ARCADE_CABINET_R,

    UI_TILE,
    UI_BANNER,
    UI_STEAMGRID,
    BACKGROUND,
    MUSIC,

    SCREENSHOTS,
    VIDEOS,
};
