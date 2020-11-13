// Pegasus Frontend
// Copyright (C) 2017-2020  Mátyás Mustoha
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

#include "utils/MoveOnly.h"

#include <QRegularExpression>
#include <vector>

namespace model { class Collection; }
namespace providers { class SearchContext; }


namespace providers {
namespace pegasus {

struct FileFilterGroup {
    std::vector<QString> extensions;
    std::vector<QString> files;
    QRegularExpression regex;

    explicit FileFilterGroup();
    MOVE_ONLY(FileFilterGroup)
};

struct FileFilter {
    //QString collection_key;
    model::Collection* collection;
    std::vector<QString> directories;
    FileFilterGroup include;
    FileFilterGroup exclude;

    explicit FileFilter(model::Collection* const, QString);
    MOVE_ONLY(FileFilter)
};

void apply_filter(FileFilter&, SearchContext&);

} // namespace pegasus
} // namespace providers
