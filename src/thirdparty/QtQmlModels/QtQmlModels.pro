TARGET = QtQmlModels
TEMPLATE = lib

QT += core qml
CONFIG += static warn_on

INCLUDEPATH += $$PWD

HEADERS += \
    $$PWD/QQmlObjectListModel.h \
    $$PWD/QQmlVariantListModel.h \
    $$PWD/QtQmlTricksPlugin_SmartDataModels.h

SOURCES += \
    $$PWD/QQmlObjectListModel.cpp \
    $$PWD/QQmlVariantListModel.cpp
