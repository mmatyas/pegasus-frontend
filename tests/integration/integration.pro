TEMPLATE = subdirs

SUBDIRS += \
    sortfilter \
    blurhash \
    apng \

!isEmpty(USE_SDL_GAMEPAD): SUBDIRS += sdl
