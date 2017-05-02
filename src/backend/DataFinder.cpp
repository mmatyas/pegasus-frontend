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


#include "DataFinder.h"

#include "Model.h"
#include "Utils.h"
#include "es2/Es2AssetFinder.h"
#include "es2/Es2Gamelist.h"
#include "es2/Es2Systems.h"

#include <QDirIterator>
#include <QStringBuilder>


QList<Model::Platform*> DataFinder::find()
{
    QList<Model::Platform*> model;

    findPlatforms(model);

    for (Model::Platform* platform : model)
        findPlatformGames(platform);
    removeEmptyPlatforms(model);

    for (const Model::Platform* platform : model) {
        findGameMetadata(*platform);
        findGameAssets(*platform);
    }

    return model;
}

void DataFinder::findPlatforms(QList<Model::Platform*>& model)
{
    Q_ASSERT(model.isEmpty());

    // at the moment, we only use ES2's platform definitions
    const QVector<Model::Platform*> es2_platforms = Es2::Systems::read();
    for (auto& platform : es2_platforms)
        model.append(platform);
}

void DataFinder::findPlatformGames(Model::Platform* platform)
{
    static const auto filters = QDir::Files | QDir::Readable | QDir::NoDotAndDotDot;
    static const auto flags = QDirIterator::Subdirectories | QDirIterator::FollowSymlinks;

    Q_ASSERT(platform);
    Q_ASSERT(!platform->m_rom_dir_path.isEmpty());
    Q_ASSERT(!platform->m_rom_filters.isEmpty());

    // TODO: handle incorrect filters
    // TODO: add proper subdirectory support

    QDirIterator romdir_it(platform->m_rom_dir_path,
                           platform->m_rom_filters,
                           filters, flags);
    while (romdir_it.hasNext())
        platform->m_games.append(new Model::Game(romdir_it.next(), platform));

    // QDir supports ordering, but doesn't support symlinks or subdirectories
    // without additional checks and recursion.
    // QDirIterator supports subdirs and symlinks, but doesn't do sorting.
    // Sorting manually should be faster than evaluating an `if dir` branch in a loop.
    std::sort(platform->m_games.begin(), platform->m_games.end(),
        [](const Model::Game* a, const Model::Game* b) {
            return QString::localeAwareCompare(a->m_rom_basename, b->m_rom_basename) < 0;
        }
    );
}

void DataFinder::removeEmptyPlatforms(QList<Model::Platform*>& platforms)
{
    // NOTE: if this turns out to be slow, STL iterators
    // could be used here
    QMutableListIterator<Model::Platform*> it(platforms);
    while (it.hasNext()) {
        if (it.next()->m_games.isEmpty())
            it.remove();
    }
}

void DataFinder::findGameMetadata(const Model::Platform& platform)
{
    Es2::Gamelist::read(platform);
}

void DataFinder::findGameAssets(const Model::Platform& platform)
{
    using Type = Assets::Type;

    for (Model::Game* game_ptr : platform.m_games) {
        Q_ASSERT(game_ptr);
        Q_ASSERT(game_ptr->m_assets);

        Model::Game& game = *game_ptr;
        Model::GameAssets& assets = *game.m_assets;

        // TODO: this should be better as a map
        // TODO: do not overwrite
        assets.m_box_front = findAsset(Type::BOX_FRONT, platform, game);
        assets.m_box_back = findAsset(Type::BOX_BACK, platform, game);
        assets.m_box_spine = findAsset(Type::BOX_SPINE, platform, game);
        assets.m_box_full = findAsset(Type::BOX_FULL, platform, game);
        assets.m_cartridge = findAsset(Type::CARTRIDGE, platform, game);
        assets.m_logo = findAsset(Type::LOGO, platform, game);
        assets.m_marquee = findAsset(Type::MARQUEE, platform, game);
        assets.m_bezel = findAsset(Type::BEZEL, platform, game);
        assets.m_gridicon = findAsset(Type::STEAMGRID, platform, game);
        assets.m_flyer = findAsset(Type::FLYER, platform, game);

        // TODO: support multiple
        assets.m_fanarts << findAsset(Type::FANARTS, platform, game);
        assets.m_screenshots << findAsset(Type::SCREENSHOTS, platform, game);
        assets.m_videos << findAsset(Type::VIDEOS, platform, game);
    }
}

QString DataFinder::findAsset(Assets::Type asset_type,
                              const Model::Platform& platform,
                              const Model::Game& game)
{
    QString path = findPortableAsset(asset_type, platform, game);
    if (!path.isEmpty())
        return path;

    // if the asset was not found in the portable paths,
    // check the compatibility modules
    path = Es2::AssetFinder::findAsset(asset_type, platform, game);
    if (!path.isEmpty())
        return path;

    return QString::null;
}

QString DataFinder::findPortableAsset(Assets::Type asset_type,
                                      const Model::Platform& platform,
                                      const Model::Game& game)
{
    Q_ASSERT(!platform.m_rom_dir_path.isEmpty());
    Q_ASSERT(!game.m_rom_basename.isEmpty());
    Q_ASSERT(Assets::suffixes.contains(asset_type));

    // check all possible [basedir] + [subdir] + [suffix] + [extension]
    // combination when searching for an asset

    const auto& possible_suffixes = Assets::suffixes[asset_type];
    const auto& possible_fileexts = Assets::extensions(asset_type);

    // check portable paths
    static const QLatin1String media_subdir("/media/");
    const QString common_subpath = platform.m_rom_dir_path
                                 % media_subdir
                                 % game.m_rom_basename;

    for (const auto& suffix : possible_suffixes) {
        for (const auto& ext : possible_fileexts) {
            const QString path = common_subpath % suffix % ext;
            if (validFile(path))
                return path;
        }
    }

    return QString::null;
}
