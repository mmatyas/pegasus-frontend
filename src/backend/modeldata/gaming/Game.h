// Pegasus Frontend
// Copyright (C) 2017-2018  Mátyás Mustoha
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

#include "GameAssets.h"
#include "utils/HashMap.h"
#include "utils/MoveOnly.h"

#include <QDateTime>
#include <QFileInfo>
#include <QString>
#include <QStringList>


namespace modeldata {

struct Game {
    explicit Game(QFileInfo fileinfo);

    const QFileInfo& fileinfo() const { return m_fileinfo; }

    QString title;
    QString summary;
    QString description;
    QString launch_cmd;
    QString launch_workdir;

    int player_count;
    bool is_favorite;
    float rating;
    QDate release_date;

    int playcount;
    qint64 playtime;
    QDateTime last_played;

    QStringList developers;
    QStringList publishers;
    QStringList genres;

    HashMap<QString, QString> extra;
    GameAssets assets;

    MOVE_ONLY(Game)

private:
    QFileInfo m_fileinfo;
};

} // namespace modeldata
