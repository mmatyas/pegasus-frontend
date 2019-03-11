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

#include "utils/MoveOnly.h"
#include "providers/Provider.h"

#include <QStringList>
#include <QRegularExpression>
#include <vector>


namespace providers {
namespace pegasus {
namespace parser {

struct FileFilterGroup {
    QStringList extensions;
    QStringList files;
    QRegularExpression regex;

    explicit FileFilterGroup();
    MOVE_ONLY(FileFilterGroup)
};

struct FileFilter {
    QString collection_key;
    QStringList directories;
    FileFilterGroup include;
    FileFilterGroup exclude;

    explicit FileFilter(QString collection, QString base_dir);
    MOVE_ONLY(FileFilter)
};

void tidy_filters(std::vector<FileFilter>&);
void process_filter(const FileFilter&, providers::SearchContext&);
void process_filters(const std::vector<FileFilter>&, providers::SearchContext&);

} // namespace parser
} // namespace pegasus
} // namespace providers
