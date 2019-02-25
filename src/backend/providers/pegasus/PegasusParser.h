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

#include "modeldata/gaming/CollectionData.h"
#include "modeldata/gaming/GameData.h"
#include "providers/Provider.h"
#include "utils/HashMap.h"
#include "utils/MoveOnly.h"

#include <QRegularExpression>
#include <QString>
#include <vector>


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
};
enum class GameAttrib : unsigned char {
    FILES,
    DEVELOPERS,
    PUBLISHERS,
    GENRES,
    PLAYER_COUNT,
    SHORT_DESC,
    LONG_DESC,
    RELEASE,
    RATING,
    LAUNCH_CMD,
    LAUNCH_WORKDIR,
    // TODO: COLLECTION,
};
// TODO: in the future
/*enum class GameFileAttrib : unsigned char {
    TITLE,
    SHORT_DESC,
    LONG_DESC,
    LAUNCH_CMD,
    LAUNCH_WORKDIR,
};*/


struct FileFilterGroup {
    QStringList extensions;
    QStringList files;
    QRegularExpression regex;

    explicit FileFilterGroup();
    MOVE_ONLY(FileFilterGroup)
};
struct FileFilter {
    // NOTE: collections can have different filtering parameters in different directories
    QString collection_name;
    QStringList directories;
    FileFilterGroup include;
    FileFilterGroup exclude;

    explicit FileFilter(QString collection, QString base_dir);
    MOVE_ONLY(FileFilter)
};


struct ParserHelpers {
    const HashMap<QString, CollAttrib> coll_attribs;
    const HashMap<QString, GameAttrib> game_attribs;
    //const HashMap<QString, GameFileAttrib> gamefile_attribs;
    const QRegularExpression rx_asset_key;
    const QRegularExpression rx_count_range;
    const QRegularExpression rx_percent;
    const QRegularExpression rx_float;
    const QRegularExpression rx_date;

    explicit ParserHelpers();
    MOVE_ONLY(ParserHelpers)
};


struct ParserContext {
    const QString metafile_path;
    const QString dir_path;

    const ParserHelpers& helpers;

    modeldata::Collection* cur_coll;
    // NOTE: while these would be highly unsafe normally, we can use the fact
    // that no games/filters are added during the time their pointer is used
    FileFilter* cur_filter;
    modeldata::Game* cur_game;


    explicit ParserContext(QString metafile_path, const ParserHelpers&);
    MOVE_ONLY(ParserContext)

    void print_error(const int lineno, const QString msg) const;
};

} // namespace parser
} // namespace pegasus
} // namespace providers
