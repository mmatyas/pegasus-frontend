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


#include "TerminalKbd.h"

#if defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)

#include <unistd.h>
#include <linux/kd.h>
#include <sys/ioctl.h>

#ifndef KDSKBMUTE
#define KDSKBMUTE 0x4B51
#endif

#ifdef K_OFF
#define KBD_OFF_MODE K_OFF
#else
#define KBD_OFF_MODE K_RAW
#endif


int TerminalKbd::m_tty = -1;
int TerminalKbd::m_old_kbd_mode = 0;

void TerminalKbd::on_startup()
{
    m_tty = ::isatty(0) ? 0 : -1;
    ::ioctl(m_tty, KDGKBMODE, &m_old_kbd_mode);
}

void TerminalKbd::enable()
{
    if (m_tty == -1)
        return;

    ::ioctl(m_tty, KDSKBMUTE, 0);
    ::ioctl(m_tty, KDSKBMODE, m_old_kbd_mode);
}

void TerminalKbd::disable()
{
    if (m_tty == -1)
        return;

    ::ioctl(m_tty, KDGKBMODE, &m_old_kbd_mode);
    ::ioctl(m_tty, KDSKBMUTE, 1);
    ::ioctl(m_tty, KDSKBMODE, KBD_OFF_MODE);
}

#else
void TerminalKbd::on_startup() {}
void TerminalKbd::enable() {}
void TerminalKbd::disable() {}
#endif
