#pragma once

#include <QString>
#include <QVector>

namespace Model { class Game; }
namespace Model { class Platform; }


namespace Es2 {

/// Compatibility class for finding game assets in ES2 directories
class Assets {
public:
    enum class AssetType : unsigned char {
        BOX_FRONT,
        BOX_BACK,
        BOX_SPINE,
        BOX_FULL,
        CARTRIDGE,
        LOGO, // just the game logo
        MARQUEE, // game logo with background
        BEZEL, // image around the game screen
        STEAMGRID,
        FLYER,
        FANARTS,
        SCREENSHOTS,
        VIDEOS,
    };

    static void findAll(const Model::Platform* platform, Model::Game* game);
    static QString find(AssetType, const Model::Platform*, const Model::Game*);

private:
    static QVector<QString> possibleSuffixes(AssetType);
    static QVector<QString> possibleExtensions(AssetType);
};

} // namespace Es2
