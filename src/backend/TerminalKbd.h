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
