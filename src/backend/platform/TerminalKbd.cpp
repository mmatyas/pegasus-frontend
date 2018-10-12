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
