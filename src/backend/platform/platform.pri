HEADERS += \
    $$PWD/PowerCommands.h \
    $$PWD/TerminalKbd.h \

SOURCES += \
    $$PWD/TerminalKbd.cpp \


win32 {
    SOURCES += $$PWD/PowerCommands_win.cpp
}
else:unix:!android {
    macx: SOURCES += $$PWD/PowerCommands_mac.cpp
    else: SOURCES += $$PWD/PowerCommands_linux.cpp
}
else {
    SOURCES += $$PWD/PowerCommands_unimpl.cpp
}

android {
    HEADERS += \
        $$PWD/AndroidAppIconProvider.h \
        $$PWD/AndroidHelpers.h

    SOURCES += \
        $$PWD/AndroidAppIconProvider.cpp \
        $$PWD/AndroidHelpers.cpp
}
