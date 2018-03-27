HEADERS += \
    platform/PowerCommands.h \

win32 {
    SOURCES += platform/PowerCommands_win.cpp
}
else:unix {
    SOURCES += platform/PowerCommands_unix.cpp
}
else {
    SOURCES += platform/PowerCommands_unimpl.cpp
}
