#pragma once

#include <QString>

#include "Assets.h"

namespace Model { class Game; }
namespace Model { class Platform; }


namespace Es2 {

/// Compatibility class for finding game assets in ES2 directories
class AssetFinder {
public:
    static QString findAsset(Assets::Type, const Model::Platform&, const Model::Game&);
};

} // namespace Es2
