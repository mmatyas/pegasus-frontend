#pragma once

#include "PlatformList.h"

#include <QString>
#include <QVector>


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
    QString short_name;
    QString long_name;
    QString rom_dir_path;
    QString launch_cmd;

    QVector<Game> games;
};

} // namespace Model
