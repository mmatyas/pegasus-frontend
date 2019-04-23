// Pegasus Frontend
// Copyright (C) 2018  Mátyás Mustoha
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

#include "GamepadAxisNavigation.h"

#include <QFile>

#ifdef Q_OS_ANDROID
#include <QGamepadKeyNavigation>
#else
#include "GamepadButtonNavigation.h"
#endif


namespace backend {

class AppContext {
public:
    AppContext();
    AppContext(const AppContext&) = delete;
    AppContext& operator=(const AppContext&) = delete;

private:
#ifdef Q_OS_ANDROID
    QGamepadKeyNavigation padkeynav;
#else
    GamepadButtonNavigation padbuttonnav;
    GamepadAxisNavigation padaxisnav;
#endif

    void setup_gamepad();
};

} // namespace backend
