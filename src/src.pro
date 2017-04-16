TEMPLATE = app

TARGET = pegasus

QT += qml quick xml
CONFIG += c++11

SOURCES += \
    backend/main.cpp \
    backend/Api.cpp \
    backend/Model.cpp \
    backend/Utils.cpp \
    backend/PlatformList.cpp \
    backend/Es2GamelistReader.cpp \
    backend/Es2Assets.cpp \
    backend/FrontendLayer.cpp \
    backend/Es2Systems.cpp

HEADERS += \
    backend/Api.h \
    backend/Model.h \
    backend/Utils.h \
    backend/PlatformList.h \
    backend/Es2GamelistReader.h \
    backend/Es2Assets.h \
    backend/FrontendLayer.h \
    backend/Es2Systems.h

RESOURCES += \
    qml.qrc \
    ../assets/assets.qrc

OTHER_FILES += \
    qml/*.qml


# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Default rules for deployment.
target.path = /tmp/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
