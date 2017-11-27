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
#include "types/Collection.h"
#include "model_providers/Es2Metadata.h"
#include "model_providers/Es2PlatformList.h"
#include "model_providers/PegasusAssets.h"

#ifndef Q_PROCESSOR_ARM
    #include "model_providers/SteamMetadata.h"
    #include "model_providers/SteamPlatform.h"
#endif

#include <QDirIterator>
#include <memory>


#include "providers/Es2Provider.h"


#include <QDebug>


namespace {
/*
QVector<Types::Collection*> runPlatformListProviders()
{
    QVector<Types::Collection*> model;

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

void findGamesByExt(Types::Collection& platform)
{
    // TODO: handle empty rom filter list

    static const auto filters = QDir::Files | QDir::Readable | QDir::NoDotAndDotDot;
    static const auto flags = QDirIterator::Subdirectories | QDirIterator::FollowSymlinks;

    // TODO: handle incorrect filters
    // TODO: add proper subdirectory support

    for (const QString& romdir : platform.searchDirs()) {
        QDirIterator romdir_it(romdir, platform.romFilters(), filters, flags);
        while (romdir_it.hasNext())
            platform.gameListMut().addGame(romdir_it.next());
    }
}
*/
void removeEmptyCollections(QVector<Types::Collection*>& platforms)
{
    // NOTE: if this turns out to be slow, STL iterators
    // could be used here
    QMutableVectorIterator<Types::Collection*> it(platforms);
    while (it.hasNext()) {
        if (it.next()->gameList().allGames().isEmpty()) {
            delete it.value();
            it.remove();
        }
    }
}

void runMetadataProviders(const Types::Collection& platform)
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

/*QVector<Types::Collection*> DataFinder::find()
{
    // TODO: map-reduce algorithms might be usable here
    // TODO: mergeDuplicatePlatforms(model);

    QVector<Types::Collection*> model = runPlatformListProviders();

    for (Types::Collection* const platform_ptr : qAsConst(model)) {
        Q_ASSERT(platform_ptr);
        Types::Collection& platform = *platform_ptr;

        findGamesByExt(platform);
    }

    removeEmptyPlatforms(model);

    for (Types::Collection* const platform_ptr : qAsConst(model)) {
        Q_ASSERT(platform_ptr);
        Types::Collection& platform = *platform_ptr;

        runMetadataProviders(platform);
        platform.gameListMut().sortGames();

        emit platformGamesReady(platform.gameList().allGames().count());
    }

    return model;
}*/

QVector<Types::Collection*> DataFinder::find()
{
    QHash<QString, Types::Game*> games;
    QHash<QString, Types::Collection*> collections;
    QVector<QString> metadata_dirs;


    using ProviderPtr = std::unique_ptr<providers::Provider>;
    std::vector<ProviderPtr> providers;

    providers.emplace_back(new providers::Es2Provider());

    for (auto& provider : providers)
        provider->find(games, collections, metadata_dirs);


    QVector<Types::Collection*> result;
    for (Types::Collection* const coll : collections) {
        qDebug()  << "coll" << coll->tag() << coll->gameList().allCount() << coll->gameList().filteredCount();
        result << coll;
}

    removeEmptyCollections(result);
    return result;
}
