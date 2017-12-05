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

#include "providers/Es2Metadata.h"
#include "providers/Es2Gamelist.h"
#include "providers/PegasusMetadata.h"
#include "types/Collection.h"

#ifndef Q_PROCESSOR_ARM
// Steam here
#endif

#include <memory>


namespace {

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

    providers.emplace_back(new providers::PegasusMetadata());
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
    for (Types::Collection* const coll : collections) {
        coll->gameListMut().sortGames();
        result << coll;
    }
    std::sort(result.begin(), result.end(),
        [](const Types::Collection* a, const Types::Collection* b) {
            return QString::localeAwareCompare(a->tag(), b->tag()) < 0;
        }
    );

    return result;
}
