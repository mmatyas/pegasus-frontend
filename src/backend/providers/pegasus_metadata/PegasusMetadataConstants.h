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


#pragma once

#include "utils/HashMap.h"
#include "utils/MoveOnly.h"

#include <QString>
#include <QRegularExpression>


namespace providers {
namespace pegasus {
namespace parser {

enum class CollAttrib : unsigned char {
    SHORT_NAME,
    DIRECTORIES,
    EXTENSIONS,
    FILES,
    REGEX,
    SHORT_DESC,
    LONG_DESC,
    LAUNCH_CMD,
    LAUNCH_WORKDIR,
    SORT_NAME,
};
enum class GameAttrib : unsigned char {
    FILES,
    DEVELOPERS,
    PUBLISHERS,
    GENRES,
    TAGS,
    PLAYER_COUNT,
    SHORT_DESC,
    LONG_DESC,
    RELEASE,
    RATING,
    LAUNCH_CMD,
    LAUNCH_WORKDIR,
    SORT_TITLE,
};
// TODO: in the future
/*enum class GameFileAttrib : unsigned char {
    TITLE,
    SHORT_DESC,
    LONG_DESC,
    LAUNCH_CMD,
    LAUNCH_WORKDIR,
};*/


struct Constants {
    const HashMap<QString, CollAttrib> coll_attribs;
    const HashMap<QString, GameAttrib> game_attribs;
    //const HashMap<QString, GameFileAttrib> gamefile_attribs;
    const QRegularExpression rx_asset_key;
    const QRegularExpression rx_count_range;
    const QRegularExpression rx_percent;
    const QRegularExpression rx_float;
    const QRegularExpression rx_date;
    const QRegularExpression rx_linebreak;

    explicit Constants();
    MOVE_ONLY(Constants)
};

} // namespace parser
} // namespace pegasus
} // namespace providers
