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

class QString;


/// Returns true if the path is an existing regular file
bool validPath(const QString& path);

/// If the string can be converted to an integer,
/// it will be saved to the provided field
void parseStoreInt(const QString& str, int& val);

/// If the string can be converted to a `float` value,
/// it will be saved to the provided field
void parseStoreFloat(const QString& str, float& val);

/// Returns $PEGASUS_HOME if defined, or $HOME if defined,
/// otherwise QDir::homePath().
QString homePath();
