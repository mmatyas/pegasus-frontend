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

#include "utils/NoCopyNoMove.h"

#include <QtGlobal>


class TerminalKbd {
public:
    static void on_startup();
    static void enable();
    static void disable();

private:
#if defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
    static int m_tty;
    static int m_old_kbd_mode;
#endif
};
