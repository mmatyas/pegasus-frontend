ENABLED_COMPATS *= "Android Apps"
USES_JSON_CACHE = yes

DEFINES *= WITH_COMPAT_ANDROIDAPPS

HEADERS += \
    $$PWD/AndroidAppsProvider.h \
    $$PWD/AndroidAppsMetadata.h \

SOURCES += \
    $$PWD/AndroidAppsProvider.cpp \
    $$PWD/AndroidAppsMetadata.cpp \
