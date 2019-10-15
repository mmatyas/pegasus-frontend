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

#include "types/AssetType.h"
#include "utils/HashMap.h"
#include "utils/NoCopyNoMove.h"

#include <QString>
#include <QXmlStreamReader>
#include <vector>


namespace providers {
namespace launchbox {

static constexpr auto MSG_PREFIX = "LaunchBox:";

using GameId = QString;
enum class GameField : unsigned char {
    ID,
    PATH,
    TITLE,
    RELEASE,
    DEVELOPER,
    PUBLISHER,
    NOTES,
    PLAYMODE,
    GENRE,
    STARS,
    EMULATOR_ID,
    EMULATOR_PARAMS,
    EMULATOR_PLATFORM,
    ASSETPATH_VIDEO,
    ASSETPATH_MUSIC,
};
enum class AdditionalAppField : unsigned char {
    ID,
    GAME_ID,
    PATH,
    NAME,
};


using EmulatorId = QString;
struct EmulatorPlatform {
    QString name;
    QString cmd_params;
};
struct Emulator {
    QString name;
    QString app_path;
    QString default_cmd_params;
    std::vector<EmulatorPlatform> platforms;
};


struct Literals {
    const HashMap<QString, GameField> gamefield_map;
    const HashMap<QString, AdditionalAppField> addiappfield_map;
    const std::vector<std::pair<QString, AssetType>> assetdir_map;

    Literals();
    NO_COPY_NO_MOVE(Literals)
};


void log_xml_warning(const QXmlStreamReader& xml, const QString& xml_rel_path, const QString& msg);
void check_lb_root_node(QXmlStreamReader& xml, const QString& xml_rel_path);

} // namespace launchbox
} // namespace providers
