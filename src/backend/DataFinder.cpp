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
#include "model_providers/Es2Metadata.h"
#include "model_providers/Es2PlatformList.h"
#include "model_providers/PegasusAssets.h"

#include <QDirIterator>


QList<Model::Platform*> DataFinder::find()
{
    // TODO: map-reduce algorithms might be usable here

    QList<Model::Platform*> model;

    findPlatforms(model);
    // TODO: mergeDuplicatePlatforms(model);

    for (Model::Platform* platform : qAsConst(model)) {
        Q_ASSERT(platform);
        findGamesByExt(*platform);
    }

    removeEmptyPlatforms(model);

    for (Model::Platform* const platform_ptr : qAsConst(model)) {
        Model::Platform& platform = *platform_ptr;
        findPortableAssets(platform);
        runMetadataProviders(platform);
        // TODO: merge duplicates?
    }

    return model;
}

void DataFinder::findPlatforms(QList<Model::Platform*>& model)
{
    Q_ASSERT(model.isEmpty());

    // If you'd like to add more than one provider, use them like this:
    //
    // QVector<model_providers::PlatformListProvider*> providers;
    // providers.push_back(new model_providers::Es2PlatformList());
    //
    // for (auto& provider : providers)
    //     model.append(provider->find());

    model_providers::Es2PlatformList provider;
    model.append(provider.find());
}

void DataFinder::findGamesByExt(Model::Platform& platform)
{
    static const auto filters = QDir::Files | QDir::Readable | QDir::NoDotAndDotDot;
    static const auto flags = QDirIterator::Subdirectories | QDirIterator::FollowSymlinks;

    // TODO: handle incorrect filters
    // TODO: add proper subdirectory support

    QDirIterator romdir_it(platform.m_rom_dir_path,
                           platform.m_rom_filters,
                           filters, flags);
    while (romdir_it.hasNext())
        platform.addGame(romdir_it.next());

    // QDir supports ordering, but doesn't support symlinks or subdirectories
    // without additional checks and recursion.
    // QDirIterator supports subdirs and symlinks, but doesn't do sorting.
    // Sorting manually should be faster than evaluating an `if dir` branch in a loop.
    platform.sortGames();
}

void DataFinder::removeEmptyPlatforms(QList<Model::Platform*>& platforms)
{
    // NOTE: if this turns out to be slow, STL iterators
    // could be used here
    QMutableListIterator<Model::Platform*> it(platforms);
    while (it.hasNext()) {
        if (it.next()->allGames().isEmpty())
            it.remove();
    }
}

void DataFinder::findPortableAssets(const Model::Platform& platform)
{
    // See `runMetadataProviders` about how to implement multiple providers.
    // You'll also need to make a new abstract base class and un-static-ify
    // PegasusAssets.
    model_providers::PegasusAssets provider;

    // TODO: this could be parallelized
    for (Model::Game* game_ptr : qAsConst(platform.allGames())) {
        Q_ASSERT(game_ptr);
        Q_ASSERT(game_ptr->assets());

        const Model::Game& game = *game_ptr;

        provider.fill(platform, game);
    }
}

void DataFinder::runMetadataProviders(const Model::Platform& platform)
{
    // At the moment there's only ES2 for metadata so I've just simply
    // created it as a regular object.
    // If you'd like to add more than one provider, use them like this:
    //
    // QVector<model_providers::MetadataProvider*> providers;
    // providers.push_back(new model_providers::Es2Metadata());
    //
    // for (auto& provider : providers)
    //     provider->fill(platform);

    model_providers::Es2Metadata provider;
    provider.fill(platform);
}
