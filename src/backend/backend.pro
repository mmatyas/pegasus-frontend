TEMPLATE = lib

QT += qml
CONFIG += c++11 staticlib warn_on exceptions_off rtti_off

SOURCES += \
    Api.cpp \
    Assets.cpp \
    DataFinder.cpp \
    Es2AssetFinder.cpp \
    Es2Gamelist.cpp \
    Es2Systems.cpp \
    FrontendLayer.cpp \
    Model.cpp \
    PlatformList.cpp \
    Utils.cpp

HEADERS += \
    Api.h \
    Assets.h \
    DataFinder.h \
    Model.h \
    PlatformList.h \
    FrontendLayer.h \
    Es2AssetFinder.h \
    Es2Gamelist.h \
    Es2Systems.h \
    Utils.h


DEFINES *= \
    QT_DEPRECATED_WARNINGS \
    QT_RESTRICTED_CAST_FROM_ASCII \
    QT_NO_CAST_TO_ASCII
