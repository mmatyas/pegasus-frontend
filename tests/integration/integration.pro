TEMPLATE = subdirs

# Proper QML testing was added in 5.11
!lessThan(QT_MAJOR_VERSION, 5): !lessThan(QT_MINOR_VERSION, 11) {
    SUBDIRS += \
        sortfilter \
        blurhash
}

!isEmpty(USE_SDL_GAMEPAD): SUBDIRS += sdl
