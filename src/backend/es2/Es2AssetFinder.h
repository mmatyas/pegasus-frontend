#pragma once

#include <QString>
#include <QVector>

#include "Assets.h"

namespace Model { class Game; }
namespace Model { class Platform; }


namespace Es2 {

/// Compatibility class for finding game assets in ES2 directories
class AssetFinder {
public:
    static void findAll(const Model::Platform& platform, Model::Game& game);
    static QString find(::Assets::Type, const Model::Platform&, const Model::Game&);
};

} // namespace Es2
