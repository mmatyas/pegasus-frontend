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

#include "utils/NoCopyNoMove.h"

#include <QString>
#include <QVector>
#include <functional>

class QFile;
class QTextStream;


namespace config {

struct Entry {
    int line;
    QString key;
    QVector<QString> values;

    void reset();
};
struct Error {
    int line;
    QString message;
};

/// Read and parse the stream, calling the callbacks when necessary.
void readStream(QTextStream& stream,
                const std::function<void(const Entry&)>& onAttributeFound,
                const std::function<void(const Error&)>& onError);

/// Opens the file at the path, then calls the stream reading on it.
/// Returns false if the file could not be opened.
bool readFile(const QString& path,
              const std::function<void(const Entry&)>& onAttributeFound,
              const std::function<void(const Error&)>& onError);

/// Calls the stream reading on an already open, readable text file.
void readFile(QFile& file,
              const std::function<void(const Entry&)>& onAttributeFound,
              const std::function<void(const Error&)>& onError);


/// Creates a single text from the separate lines. Lines are expected to be
/// trimmed, or contain only a single line break.
QString mergeLines(const QVector<QString>&);

} // namespace config
