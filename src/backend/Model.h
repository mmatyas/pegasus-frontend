#pragma once

#include "PlatformList.h"

#include <QString>
#include <QVector>


namespace Model {

struct GameAssets {
    QString box_front_path;
};

struct Game {
    QString path;

    QString title;
    QString description;
    QString developer;

    GameAssets assets;
};

struct Platform {
    QString short_name;
    QString long_name;
    QString rom_dir_path;
    QString launch_cmd;

    QVector<Game> games;
};

} // namespace Model
