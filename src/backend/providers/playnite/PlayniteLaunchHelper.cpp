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

#include "PlayniteLaunchHelper.h"

#include "PlayniteComponents.h"
#include "providers/ProviderUtils.h"
#include "utils/CommandTokenizer.h"

#include <QDir>

namespace providers {
namespace playnite {

namespace {
QString build_launch_cmd(const QString& exec_path, const QString& args)
{
    return QStringLiteral("%1 %2").arg(QDir::cleanPath(exec_path), args);
}
} // namespace

PlayniteLaunchHelper::PlayniteLaunchHelper()
    : m_steam_call(providers::find_steam_call())
{}

PlayniteGameLaunchInfo::LaunchType PlayniteLaunchHelper::get_launch_type_enum(const int type)
{
    switch(type) {
        case 0:
            return PlayniteGameLaunchInfo::LaunchType::INSTALLED;
        case 1:
            return PlayniteGameLaunchInfo::LaunchType::STEAM;
        case 2:
            return PlayniteGameLaunchInfo::LaunchType::EMULATOR;
        default:
            return PlayniteGameLaunchInfo::LaunchType::UNKNOWN;
    }
}

PlayniteGameLaunchInfo PlayniteLaunchHelper::get_game_launch_info(
    const PlayniteGame& game_info,
    const PlayniteComponents& components) const
{
    const PlayniteGame::PlayAction& play_action = game_info.play_action;
    const PlayniteGameLaunchInfo::LaunchType& launch_type = get_launch_type_enum(play_action.type);
    QString path;
    QString working_dir;
    QString launch_cmd;

    if (launch_type == PlayniteGameLaunchInfo::LaunchType::INSTALLED) {
        path = QStringLiteral("%1/%2").arg(game_info.install_directory, play_action.path);
        working_dir = play_action.working_dir;
        if (working_dir == QLatin1String("{InstallDir}"))
            working_dir = game_info.install_directory;
        launch_cmd = build_launch_cmd(utils::escape_command(path), play_action.arguments);
    }
    else if (launch_type == PlayniteGameLaunchInfo::LaunchType::STEAM) {
        const QString args = QStringLiteral("%1 %2").arg(play_action.path, play_action.arguments);
        launch_cmd = build_launch_cmd(m_steam_call, args);
    }
    else if (launch_type == PlayniteGameLaunchInfo::LaunchType::EMULATOR) {
        const PlayniteEmulator& emulator = components.emulators.at(play_action.emulator_id);
        const PlayniteEmulator::EmulatorProfile& emu_profile = emulator.profiles.at(play_action.emulator_profile_id);
        QString args = emu_profile.arguments;
        args.replace(QStringLiteral("\"{ImagePath}\""), QStringLiteral("{file.path}"));
        const QString exec_path = emu_profile.executable;
        path = game_info.game_image_path;
        working_dir = emu_profile.working_dir;
        launch_cmd = build_launch_cmd(utils::escape_command(exec_path), args);
    }

    PlayniteGameLaunchInfo launch_info;
    launch_info.type = launch_type;
    launch_info.path = QDir::cleanPath(path);
    launch_info.working_dir = QDir::cleanPath(working_dir);
    launch_info.launch_cmd = launch_cmd;
    return launch_info;
}

} // namespace playnite
} // namespace providers
