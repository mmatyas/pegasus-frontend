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


#include "GameFileData.h"


namespace modeldata {

QString pretty_filename(const QFileInfo& fi)
{
    return fi.completeBaseName()
        .replace(QLatin1Char('_'), QLatin1Char(' '))
        .replace(QLatin1Char('.'), QLatin1Char(' '));
}


GameFile::GameFile(QFileInfo fi)
    : fileinfo(std::move(fi))
    , name(pretty_filename(fileinfo))
    , play_time(0)
    , play_count(0)
{}

bool GameFile::operator==(const GameFile& other) const {
    return fileinfo == other.fileinfo;
}
} // namespace modeldata
