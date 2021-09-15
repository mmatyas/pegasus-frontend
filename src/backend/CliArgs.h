// Pegasus Frontend
// Copyright (C) 2020  Mátyás Mustoha
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

namespace backend {
struct CliArgs {
    bool portable = false;
    bool silent = false;
    bool enable_menu_appclose = true;
    bool enable_menu_shutdown = true;
    bool enable_menu_suspend = true;
    bool enable_menu_reboot = true;
    bool enable_menu_settings = true;
    bool enable_menu_help = false;
    bool enable_gamepad_autoconfig = true;
};
} // namespace backend
