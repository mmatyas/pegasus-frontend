#pragma once

#include <QHash>
#include <QString>


// The following list is based on EmulationStation's similar code

namespace Platforms {

enum Id {
    UNKNOWN = 0,
    THREEDO,
    AMIGA,
    AMSTRAD_CPC,
    APPLE_II,
    ARCADE,
    ATARI_800,
    ATARI_2600,
    ATARI_5200,
    ATARI_7800,
    ATARI_LYNX,
    ATARI_ST, // Atari ST/STE/Falcon
    ATARI_JAGUAR,
    ATARI_JAGUAR_CD,
    ATARI_XE,
    COLECOVISION,
    COMMODORE_64,
    INTELLIVISION,
    MAC_OS,
    XBOX,
    XBOX_360,
    XBOX_ONE,
    MSX,
    NEOGEO,
    NEOGEO_POCKET,
    NEOGEO_POCKET_COLOR,
    NINTENDO_3DS,
    NINTENDO_64,
    NINTENDO_DS,
    NINTENDO_GAME_BOY,
    NINTENDO_GAME_BOY_ADVANCE,
    NINTENDO_GAME_BOY_COLOR,
    NINTENDO_GAMECUBE,
    NINTENDO_NES,
    NINTENDO_SNES,
    NINTENDO_SWITCH,
    NINTENDO_WII,
    NINTENDO_WII_U,
    PC,
    SEGA_32X,
    SEGA_CD,
    SEGA_DREAMCAST,
    SEGA_GAME_GEAR,
    SEGA_GENESIS,
    SEGA_MASTER_SYSTEM,
    SEGA_MEGA_DRIVE,
    SEGA_SATURN,
    PLAYSTATION,
    PLAYSTATION_2,
    PLAYSTATION_3,
    PLAYSTATION_4,
    PLAYSTATION_VITA,
    PLAYSTATION_PORTABLE,
    TURBOGRAFX_16, // also known as PC Engine
    WONDERSWAN,
    WONDERSWAN_COLOR,
    ZX_SPECTRUM,
};

QString shortName(Id);
QString longName(Id);

Id idFromShortName(const QString&);

} // namespace Platforms
