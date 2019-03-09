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

#include "PegasusMetadataConstants.h"
#include "modeldata/gaming/CollectionData.h"
#include "modeldata/gaming/GameData.h"
#include "utils/MoveOnly.h"

#include <QString>
#include <vector>

namespace providers { namespace pegasus { namespace filter {
class FileFilter;
}}}


namespace providers {
namespace pegasus {
namespace parser {

struct ParserContext {
    const QString metafile_path;
    const QString dir_path;

    const Constants& constants;

    modeldata::Collection* cur_coll;
    // NOTE: while these would be highly unsafe normally, we can use the fact
    // that no games/filters are added during the time their pointer is used
    filter::FileFilter* cur_filter;
    modeldata::Game* cur_game;


    explicit ParserContext(QString metafile_path, const Constants&);
    MOVE_ONLY(ParserContext)

    void print_error(const int lineno, const QString msg) const;
};

} // namespace parser
} // namespace pegasus
} // namespace providers
