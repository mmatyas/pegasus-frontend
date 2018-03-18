// Pegasus Frontend
// Copyright (C) 2017  Mátyás Mustoha
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

#include <QString>
#include <QTextStream>
#include <functional>


namespace config {

/// Read and parse the stream, calling the callbacks if necessary
/// - onAttributeFound(line, key, value)
/// - onError(line, message)
void readStream(QTextStream& stream,
                const std::function<void(const int, const QString, const QString)>& onAttributeFound,
                const std::function<void(const int, const QString)>& onError);

/// Opens the file at the path, then calls the stream reading on it
void readFile(const QString& path,
              const std::function<void(const int, const QString, const QString)>& onAttributeFound,
              const std::function<void(const int, const QString)>& onError);

} // namespace config
