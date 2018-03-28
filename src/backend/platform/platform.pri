
HEADERS += \
    platform/PowerCommands.h \

win32 {
    SOURCES += platform/PowerCommands_win.cpp
    LIBS += -luser32 -ladvapi32
}
else:unix {
    SOURCES += platform/PowerCommands_unix.cpp
    qtHaveModule(dbus): QT += dbus
}
else {
    SOURCES += platform/PowerCommands_unimpl.cpp
}
