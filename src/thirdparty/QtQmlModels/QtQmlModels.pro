TARGET = QtQmlModels
TEMPLATE = lib

QT += core qml
CONFIG += static warn_on

INCLUDEPATH += $$PWD

HEADERS += $$PWD/QQmlObjectListModel.h
SOURCES += $$PWD/QQmlObjectListModel.cpp
