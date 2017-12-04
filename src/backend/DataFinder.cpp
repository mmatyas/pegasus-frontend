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


#include "providers/Es2Metadata.h"
#include "providers/Es2Gamelist.h"


#include <QDebug>


namespace {
/*
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
void removeEmptyCollections(QHash<QString, Types::Collection*>& collections)
{
    // NOTE: if this turns out to be slow, STL iterators
    // could be used here
    QMutableHashIterator<QString, Types::Collection*> it(collections);
    while (it.hasNext()) {
        if (it.next().value()->gameList().allGames().isEmpty()) {
            delete it.value();
            it.remove();
        }
    }
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

// Providers can add new games, new collections and further directories
// to check for metadata info.
void DataFinder::runListProviders(QHash<QString, Types::Game*>& games,
                                  QHash<QString, Types::Collection*>& collections,
                                  QVector<QString>& metadata_dirs)
{
    using ProviderPtr = std::unique_ptr<providers::GamelistProvider>;
    std::vector<ProviderPtr> providers;

    providers.emplace_back(new providers::Es2Provider());

    int total_game_count = 0;
    for (auto& provider : providers) {
        provider->find(games, collections, metadata_dirs);
        if (total_game_count != games.count()) {
            total_game_count = games.count();
            emit totalCountChanged(total_game_count);
        }
    }

    removeEmptyCollections(collections);
}

void DataFinder::runMetadataProviders(const QHash<QString, Types::Game*>& games,
                                      const QHash<QString, Types::Collection*>& collections,
                                      const QVector<QString>& metadata_dirs)
{
    using ProviderPtr = std::unique_ptr<providers::MetadataProvider>;
    std::vector<ProviderPtr> providers;

    providers.emplace_back(new providers::Es2Metadata());

    for (auto& provider : qAsConst(providers))
        provider->fill(games, collections, metadata_dirs);
}

QVector<Types::Collection*> DataFinder::find()
{
    QHash<QString, Types::Game*> games;
    QHash<QString, Types::Collection*> collections;
    QVector<QString> metadata_dirs;


    runListProviders(games, collections, metadata_dirs);
    runMetadataProviders(games, collections, metadata_dirs);


    QVector<Types::Collection*> result;
    for (Types::Collection* const coll : collections)
        result << coll;

    return result;
}
