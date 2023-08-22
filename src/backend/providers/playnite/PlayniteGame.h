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

#include <QString>
#include <QStringList>

namespace providers {
namespace playnite {

struct PlayniteGame {
    struct PlayAction {
        int type;
        QString path;
        QString arguments;
        QString working_dir;
        QString emulator_id;
        QString emulator_profile_id;
    };

    QString background_image;
    QString description;
    QStringList genre_ids;
    QString cover_image;
    QString game_id;
    QString platform_id;
    QStringList publisher_ids;
    QStringList developer_ids;
    QString release_date;
    float community_score = 0.0f;
    QString id;
    QString name;
    QString source_id;
    QString install_directory;
    QString game_image_path;
    bool installed = false;
    bool hidden = false;
    PlayAction play_action;
};

} // namespace playnite
} // namespace providers
