#pragma once

#include <QString>
#include <QVector>

namespace Model { class Game; }
namespace Model { class Platform; }


class Es2Assets {
public:
    enum class AssetType : unsigned char {
        BOX_FRONT,
        BOX_BACK,
        BOX_SPINE,
        BOX_FULL,
        LOGO, // just the game logo
        MARQUEE, // game logo with background
        BEZEL, // image around the game screen
        STEAMGRID,
        FANARTS,
        FLYERS,
        SCREENSHOTS,
        VIDEOS,
    };

    static QString find(AssetType, const Model::Platform*, const Model::Game*);

private:
    static QVector<QString> possibleSuffixes(AssetType);
    static QVector<QString> possibleExtensions(AssetType);
};
