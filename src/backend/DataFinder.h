// Pegasus Frontend
// Copyright (C) 2017  Mátyás Mustoha
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.


#pragma once

#include "Assets.h"

#include <QList>

namespace Model { class Game; }
namespace Model { class Platform; }


// TODO: make this a QObject with progress signals
class DataFinder {
public:
    static QList<Model::Platform*> find();

private:
    static void findPlatforms(QList<Model::Platform*>&);
    static void findGamesByExt(Model::Platform&);
    static void removeEmptyPlatforms(QList<Model::Platform*>&);

    static void findGameMetadata(const Model::Platform&);
    static void findGameAssets(const Model::Platform&);

    static QString findAsset(Assets::Type asset_type,
                             const Model::Platform& platform,
                             const Model::Game& game);
    static QString findPortableAsset(Assets::Type asset_type,
                                     const Model::Platform& platform,
                                     const Model::Game& game);
};
