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

#include "model/Platform.h"
#include "model_providers/Es2Metadata.h"
#include "model_providers/Es2PlatformList.h"
#include "model_providers/PegasusAssets.h"

#include <QDirIterator>

#include <memory>


QList<Model::Platform*> DataFinder::find()
{
    // TODO: map-reduce algorithms might be usable here
    // TODO: mergeDuplicatePlatforms(model);

    QList<Model::Platform*> model = runPlatformListProviders();

    for (Model::Platform* const platform_ptr : qAsConst(model)) {
        Q_ASSERT(platform_ptr);
        Model::Platform& platform = *platform_ptr;

        findGamesByExt(platform);
    }

    removeEmptyPlatforms(model);

    for (Model::Platform* const platform_ptr : qAsConst(model)) {
        Q_ASSERT(platform_ptr);
        Model::Platform& platform = *platform_ptr;

        runMetadataProviders(platform);
        platform.sortGames();
    }

    return model;
}

QList<Model::Platform*> DataFinder::runPlatformListProviders()
{
    QList<Model::Platform*> model;

    // If you'd like to add more than one provider, use them like this:
    //
    // QVector<model_providers::PlatformListProvider*> providers;
    // providers.push_back(new model_providers::Es2PlatformList());
    //
    // for (auto& provider : providers)
    //     model.append(provider->find());

    model_providers::Es2PlatformList provider;
    model.append(provider.find());

    return model;
}

void DataFinder::findGamesByExt(Model::Platform& platform)
{
    // TODO: handle empty rom filter list

    static const auto filters = QDir::Files | QDir::Readable | QDir::NoDotAndDotDot;
    static const auto flags = QDirIterator::Subdirectories | QDirIterator::FollowSymlinks;

    // TODO: handle incorrect filters
    // TODO: add proper subdirectory support

    QDirIterator romdir_it(platform.m_rom_dir_path,
                           platform.m_rom_filters,
                           filters, flags);
    while (romdir_it.hasNext())
        platform.addGame(romdir_it.next());
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

void DataFinder::runMetadataProviders(const Model::Platform& platform)
{
    // NOTE: Qt containers have some troubles with move semantics
    // TODO: do not recreate the providers for each platform every time

    using ProviderPtr = std::unique_ptr<model_providers::MetadataProvider>;
    std::vector<ProviderPtr> providers;

    providers.emplace_back(new model_providers::PegasusAssets());
    providers.emplace_back(new model_providers::Es2Metadata());

    for (auto& provider : qAsConst(providers))
        provider->fill(platform);
}
