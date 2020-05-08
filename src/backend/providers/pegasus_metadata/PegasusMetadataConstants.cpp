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


#include "PegasusMetadataConstants.h"


namespace providers {
namespace pegasus {
namespace parser {

Constants::Constants()
    : coll_attribs {
        { QStringLiteral("shortname"), CollAttrib::SHORT_NAME },
        { QStringLiteral("launch"), CollAttrib::LAUNCH_CMD },
        { QStringLiteral("command"), CollAttrib::LAUNCH_CMD },
        { QStringLiteral("workdir"), CollAttrib::LAUNCH_WORKDIR },
        { QStringLiteral("cwd"), CollAttrib::LAUNCH_WORKDIR },
        { QStringLiteral("directory"), CollAttrib::DIRECTORIES },
        { QStringLiteral("directories"), CollAttrib::DIRECTORIES },
        { QStringLiteral("extension"), CollAttrib::EXTENSIONS },
        { QStringLiteral("extensions"), CollAttrib::EXTENSIONS },
        { QStringLiteral("file"), CollAttrib::FILES },
        { QStringLiteral("files"), CollAttrib::FILES },
        { QStringLiteral("regex"), CollAttrib::REGEX },
        { QStringLiteral("ignore-extension"), CollAttrib::EXTENSIONS },
        { QStringLiteral("ignore-extensions"), CollAttrib::EXTENSIONS },
        { QStringLiteral("ignore-file"), CollAttrib::FILES },
        { QStringLiteral("ignore-files"), CollAttrib::FILES },
        { QStringLiteral("ignore-regex"), CollAttrib::REGEX },
        { QStringLiteral("summary"), CollAttrib::SHORT_DESC },
        { QStringLiteral("description"), CollAttrib::LONG_DESC },
        // sort name variations
        { QStringLiteral("sortname"), CollAttrib::SORT_NAME },
        { QStringLiteral("sort_name"), CollAttrib::SORT_NAME },
        { QStringLiteral("sort-name"), CollAttrib::SORT_NAME },
    }
    , game_attribs {
        { QStringLiteral("file"), GameAttrib::FILES },
        { QStringLiteral("files"), GameAttrib::FILES },
        { QStringLiteral("launch"), GameAttrib::LAUNCH_CMD },
        { QStringLiteral("command"), GameAttrib::LAUNCH_CMD },
        { QStringLiteral("workdir"), GameAttrib::LAUNCH_WORKDIR },
        { QStringLiteral("cwd"), GameAttrib::LAUNCH_WORKDIR },
        { QStringLiteral("developer"), GameAttrib::DEVELOPERS },
        { QStringLiteral("developers"), GameAttrib::DEVELOPERS },
        { QStringLiteral("publisher"), GameAttrib::PUBLISHERS },
        { QStringLiteral("publishers"), GameAttrib::PUBLISHERS },
        { QStringLiteral("genre"), GameAttrib::GENRES },
        { QStringLiteral("genres"), GameAttrib::GENRES },
        { QStringLiteral("tag"), GameAttrib::TAGS },
        { QStringLiteral("tags"), GameAttrib::TAGS },
        { QStringLiteral("players"), GameAttrib::PLAYER_COUNT },
        { QStringLiteral("summary"), GameAttrib::SHORT_DESC },
        { QStringLiteral("description"), GameAttrib::LONG_DESC },
        { QStringLiteral("release"), GameAttrib::RELEASE },
        { QStringLiteral("rating"), GameAttrib::RATING },
        // sort title variations
        { QStringLiteral("sorttitle"), GameAttrib::SORT_TITLE },
        { QStringLiteral("sortname"), GameAttrib::SORT_TITLE },
        { QStringLiteral("sort_title"), GameAttrib::SORT_TITLE },
        { QStringLiteral("sort_name"), GameAttrib::SORT_TITLE },
        { QStringLiteral("sort-title"), GameAttrib::SORT_TITLE },
        { QStringLiteral("sort-name"), GameAttrib::SORT_TITLE },
    }
    /*, gamefile_attribs {
        { QStringLiteral("name"), GameFileAttrib::TITLE },
        { QStringLiteral("title"), GameFileAttrib::TITLE },
        { QStringLiteral("launch"), GameFileAttrib::LAUNCH_CMD },
        { QStringLiteral("command"), GameFileAttrib::LAUNCH_CMD },
        { QStringLiteral("workdir"), GameFileAttrib::LAUNCH_WORKDIR },
        { QStringLiteral("cwd"), GameFileAttrib::LAUNCH_WORKDIR },
        { QStringLiteral("summary"), GameFileAttrib::SHORT_DESC },
        { QStringLiteral("description"), GameFileAttrib::LONG_DESC },
    }*/
    , rx_asset_key(QStringLiteral(R"(^assets?\.(.+)$)"))
    , rx_count_range(QStringLiteral("^(\\d+)(-(\\d+))?$"))
    , rx_percent(QStringLiteral("^\\d+%$"))
    , rx_float(QStringLiteral("^\\d(\\.\\d+)?$"))
    , rx_date(QStringLiteral("^(\\d{4})(-(\\d{1,2}))?(-(\\d{1,2}))?$"))
    , rx_linebreak(QStringLiteral(R"((?<!\\)\\n)"))
{}

} // namespace parser
} // namespace pegasus
} // namespace providers
