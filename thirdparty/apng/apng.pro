TARGET = qapng
TEMPLATE = lib
CONFIG += plugin static warn_on
QT += gui

DEFINES *= $${COMMON_DEFINES}

HEADERS += \
    apngimageplugin.h \
    apngreader_p.h \
    apngimagehandler_p.h

SOURCES += \
    apngimageplugin.cpp \
    apngimagehandler.cpp \
    apngreader.cpp

OTHER_FILES += \
    qapng.json
