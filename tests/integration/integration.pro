TEMPLATE = subdirs

SUBDIRS += \
    sortfilter \

!isEmpty(USE_SDL_GAMEPAD): SUBDIRS += sdl_gamepad
!isEmpty(ENABLE_APNG): SUBDIRS += apng
