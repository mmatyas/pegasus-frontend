#pragma once

#include "Platforms.h"

#include <QString>


namespace Model {

struct GameAssets {
    const QString box_front;
};

struct Game {
    QString filename;

    QString title;
    QString description;

    GameAssets assets;
};

struct Platform {
    Platforms::Id id;
    QString rom_dir_path;
    QString launch_cmd;

    Platform();
};

} // namespace Model
