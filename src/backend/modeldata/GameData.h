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

#include "GameAssetsData.h"
#include "GameFileData.h"
#include "utils/MoveOnly.h"

#include <QDateTime>
#include <QFileInfo>
#include <QString>
#include <QStringList>
#include <vector>


namespace modeldata {

struct Game {
    explicit Game(QFileInfo);
    explicit Game(QString);
    MOVE_ONLY(Game)

    QString title;
    QString summary;
    QString description;

    QString launch_cmd;
    QString launch_workdir;
    QString relative_basedir;
    std::vector<GameFile> files;

    short player_count;
    bool is_favorite;
    float rating;
    QDate release_date;

    QStringList developers;
    QStringList publishers;
    QStringList genres;
    QStringList tags;

    GameAssets assets;
};

} // namespace modeldata
