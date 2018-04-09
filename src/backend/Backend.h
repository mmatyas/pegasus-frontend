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

#include "Api.h"
#include "FrontendLayer.h"
#include "GamepadAxisNavigation.h"
#include "ProcessLauncher.h"

#include <QFile>
#include <QGamepadKeyNavigation>
#include <QTextStream>
#include <vector>


namespace backend {

class Context {
public:
    Context();
    ~Context();
    Context(const Context&) = delete;
    Context& operator=(const Context&) = delete;

private:
    QFile logfile;
    QGamepadKeyNavigation padkeynav;
    GamepadAxisNavigation padaxisnav;

    void setup_logging();
    void setup_gamepad();
};

class Backend {
public:
    Backend();
    Backend(const Context&) = delete;
    Backend& operator=(const Context&) = delete;

    void start();

private:
    // frontend <-> api <-> launcher
    ApiObject api;
    FrontendLayer frontend;
    ProcessLauncher launcher;
};

} // namespace backend
