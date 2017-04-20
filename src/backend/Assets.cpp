#include "Assets.h"


const QHash<Assets::Type, QVector<QString>> Assets::suffixes = {
    { Assets::Type::BOX_FRONT, { "-boxFront", "-box_front", "-boxart2D", "" } },
    { Assets::Type::BOX_BACK, { "-boxBack", "-box_back" } },
    { Assets::Type::BOX_SPINE, { "-boxSpine", "-box_spine", "-boxSide", "-box_side" } },
    { Assets::Type::BOX_FULL, { "-boxFull", "-box_full", "-box" } },
    { Assets::Type::CARTRIDGE, { "-cartridge", "-cart", "-disc" } },
    { Assets::Type::LOGO, { "-logo", "-wheel" } },
    { Assets::Type::MARQUEE, { "-marquee" } },
    { Assets::Type::BEZEL, { "-bezel", "-screenmarquee", "-border" } },
    { Assets::Type::STEAMGRID, { "-steam", "-steamgrid", "-grid" } },
    { Assets::Type::FANARTS, { "-fanart", "-art" } },
    { Assets::Type::FLYER, { "-flyer" } },
    { Assets::Type::SCREENSHOTS, { "-screenshot" } },
    { Assets::Type::VIDEOS, { "-video" } },
};

#ifdef Q_PROCESSOR_ARM
// prefer opaque images on embedded systems
const QVector<QString> Assets::image_exts = { ".jpg", ".png" };
#else
const QVector<QString> Assets::image_exts = { ".png", ".jpg" };
#endif

const QVector<QString> Assets::video_exts = { ".webm", ".mp4", ".avi" };
