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

#include "types/Game.h"
#include "types/Platform.h"
#include "model_providers/Es2Metadata.h"
#include "model_providers/Es2PlatformList.h"
#include "model_providers/PegasusAssets.h"

#ifndef Q_PROCESSOR_ARM
    #include "model_providers/SteamMetadata.h"
    #include "model_providers/SteamPlatform.h"
#endif

#include <QDirIterator>
#include <memory>


namespace {

QVector<Types::Platform*> runPlatformListProviders()
{
    QVector<Types::Platform*> model;

    using ProviderPtr = std::unique_ptr<model_providers::PlatformListProvider>;
    std::vector<ProviderPtr> providers;

    providers.emplace_back(new model_providers::Es2PlatformList());
#ifndef Q_PROCESSOR_ARM
    providers.emplace_back(new model_providers::SteamPlatform());
#endif

    for (auto& provider : qAsConst(providers))
        model.append(provider->find());

    return model;
}

void findGamesByExt(Types::Platform& platform)
{
    // TODO: handle empty rom filter list

    static const auto filters = QDir::Files | QDir::Readable | QDir::NoDotAndDotDot;
    static const auto flags = QDirIterator::Subdirectories | QDirIterator::FollowSymlinks;

    // TODO: handle incorrect filters
    // TODO: add proper subdirectory support

    for (const QString& romdir : platform.m_rom_dirs) {
        QDirIterator romdir_it(romdir, platform.m_rom_filters, filters, flags);
        while (romdir_it.hasNext())
            platform.gameListMut().addGame(romdir_it.next());
    }
}

void removeEmptyPlatforms(QVector<Types::Platform*>& platforms)
{
    // NOTE: if this turns out to be slow, STL iterators
    // could be used here
    QMutableVectorIterator<Types::Platform*> it(platforms);
    while (it.hasNext()) {
        if (it.next()->gameList().allGames().isEmpty()) {
            delete it.value();
            it.remove();
        }
    }
}

void runMetadataProviders(const Types::Platform& platform)
{
    // NOTE: Qt containers have some troubles with move semantics
    // TODO: do not recreate the providers for each platform every time

    using ProviderPtr = std::unique_ptr<model_providers::MetadataProvider>;
    std::vector<ProviderPtr> providers;

    providers.emplace_back(new model_providers::PegasusAssets());
    providers.emplace_back(new model_providers::Es2Metadata());
#ifndef Q_PROCESSOR_ARM
    providers.emplace_back(new model_providers::SteamMetadata());
#endif

    for (auto& provider : qAsConst(providers))
        provider->fill(platform);
}

} // namespace


DataFinder::DataFinder(QObject* parent)
    : QObject(parent)
{}

QVector<Types::Platform*> DataFinder::find()
{
    // TODO: map-reduce algorithms might be usable here
    // TODO: mergeDuplicatePlatforms(model);

    QVector<Types::Platform*> model = runPlatformListProviders();

    for (Types::Platform* const platform_ptr : qAsConst(model)) {
        Q_ASSERT(platform_ptr);
        Types::Platform& platform = *platform_ptr;

        findGamesByExt(platform);
    }

    removeEmptyPlatforms(model);

    for (Types::Platform* const platform_ptr : qAsConst(model)) {
        Q_ASSERT(platform_ptr);
        Types::Platform& platform = *platform_ptr;

        runMetadataProviders(platform);
        platform.gameListMut().sortGames();

        emit platformGamesReady(platform.gameList().allGames().count());
    }

    return model;
}
