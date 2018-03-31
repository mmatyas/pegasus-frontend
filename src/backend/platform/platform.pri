HEADERS += \
    platform/PowerCommands.h \

win32 {
    SOURCES += platform/PowerCommands_win.cpp
}
else:unix: {
    macx: SOURCES += platform/PowerCommands_mac.cpp
    else: SOURCES += platform/PowerCommands_linux.cpp
}
else {
    SOURCES += platform/PowerCommands_unimpl.cpp
}
