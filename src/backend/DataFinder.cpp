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

#include "providers/es2/Es2Provider.h"
//#include "providers/PegasusMetadata.h"
#include "types/Collection.h"

#ifndef Q_PROCESSOR_ARM
//#include "providers/SteamGamelist.h"
//#include "providers/SteamMetadata.h"
#endif


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
{
    providers.emplace_back(new providers::es2::Es2Provider());
#ifndef Q_PROCESSOR_ARM
    //providers.emplace_back(new providers::SteamGamelist());
#endif

    for (auto& provider : providers) {
        connect(dynamic_cast<QObject*>(provider.get()), SIGNAL(gameCountChanged(int)),
                this, SIGNAL(totalCountChanged(int)));
    }
}

// Providers can add new games, new collections and further directories
// to check for metadata info.
void DataFinder::runListProviders(QHash<QString, Types::Game*>& games,
                                  QHash<QString, Types::Collection*>& collections)
{
    for (auto& provider : providers)
        provider->find(games, collections);

    removeEmptyCollections(collections);
}

void DataFinder::runMetadataProviders(const QHash<QString, Types::Game*>& games,
                                      const QHash<QString, Types::Collection*>& collections)
{
    emit metadataSearchStarted();

    for (auto& provider : qAsConst(providers))
        provider->enhance(games, collections);
}

QVector<Types::Collection*> DataFinder::find()
{
    QHash<QString, Types::Game*> games;
    QHash<QString, Types::Collection*> collections;


    runListProviders(games, collections);
    runMetadataProviders(games, collections);


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
