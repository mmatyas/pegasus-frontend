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

namespace providers {
namespace playnite {

struct PlayniteGame;
struct PlayniteComponents;

struct PlayniteGameLaunchInfo {
    enum class LaunchType {
        STEAM,
        INSTALLED,
        EMULATOR,
        UNKNOWN,
    };

    LaunchType type = LaunchType::UNKNOWN;
    QString path;
    QString working_dir;
    QString launch_cmd;
};

class PlayniteLaunchHelper {
public:
    PlayniteLaunchHelper();
    PlayniteGameLaunchInfo get_game_launch_info(const PlayniteGame& game_info, const PlayniteComponents& components) const;
    static PlayniteGameLaunchInfo::LaunchType get_launch_type_enum(int type);

private:
    const QString m_steam_call;
};

} // namespace playnite
} // namespace providers
