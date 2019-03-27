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

#include "providers/Provider.h"
#include "utils/FwdDeclModelData.h"
#include "utils/MoveOnly.h"

#include <QString>
#include <vector>

namespace config { struct Entry; }


namespace providers {
namespace pegasus {
namespace parser {

struct Constants;
struct FileFilter;


class Parser {
public:
    explicit Parser(QString metafile_path, const Constants&);
    MOVE_ONLY(Parser)

    void print_error(const int lineno, const QString& msg) const;
    void parse_entry(const config::Entry&, providers::SearchContext&, std::vector<FileFilter>&);

private:
    const QString m_metafile_path;
    const QString m_dir_path;

    const Constants& m_constants;

    // NOTE: while these would be highly unsafe normally, we can use the fact
    // that no games/filters are added during the time their pointer is used
    modeldata::Collection* m_cur_coll;
    modeldata::Game* m_cur_game;
    FileFilter* m_cur_filter;

private:
    void parse_collection_entry(const config::Entry&) const;
    void parse_game_entry(const config::Entry&, providers::SearchContext&) const;
    bool parse_asset_entry_maybe(const config::Entry&) const;
};

} // namespace parser
} // namespace pegasus
} // namespace providers
