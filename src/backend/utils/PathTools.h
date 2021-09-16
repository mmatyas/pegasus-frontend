// Pegasus Frontend
// Copyright (C) 2017-2021  Mátyás Mustoha
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

class QString;
class QFileInfo;

/// Returns a cleaned absoluteFilePath
QString clean_abs_path(const QFileInfo&);
/// Returns a cleaned absolutePath
QString clean_abs_dir(const QFileInfo&);
/// Returns a displayable absoluteFilePath
QString pretty_path(const QFileInfo&);
/// Returns a displayable absolutePath
QString pretty_dir(const QFileInfo&);
/// Returns a displayable path
QString pretty_path(const QString&);

template <typename T>
void pretty_dir(T) = delete;
