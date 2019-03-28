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

#include <QString>
#include <vector>
#include <functional>

class QFile;
class QTextStream;


namespace metafile {

struct Entry {
    size_t line;
    QString key;
    std::vector<QString> values;

    void reset();
    MOVE_ONLY(Entry)
};
struct Error {
    size_t line;
    QString message;

    MOVE_ONLY(Error)
};


void read_stream(QTextStream& stream,
                 const std::function<void(const Entry&)>& onAttributeFound,
                 const std::function<void(const Error&)>& onError);

bool read_file(const QString& path,
               const std::function<void(const Entry&)>& onAttributeFound,
               const std::function<void(const Error&)>& onError);

void read_file(QFile& file,
               const std::function<void(const Entry&)>& onAttributeFound,
               const std::function<void(const Error&)>& onError);


QString merge_lines(const std::vector<QString>&);

} // namespace metafile
