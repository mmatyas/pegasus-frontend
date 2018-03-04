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

#include "PegasusAssets.h"
#include "providers/es2/Es2Provider.h"
#include "providers/pegasus/PegasusProvider.h"
#include "types/Collection.h"

#ifndef Q_PROCESSOR_ARM
#include "providers/steam/SteamProvider.h"
#endif

#include <QDebug>


namespace {

void removeEmptyCollections(QHash<QString, Types::Collection*>& collections)
{
    // NOTE: if this turns out to be slow, STL iterators
    // could be used here
    QMutableHashIterator<QString, Types::Collection*> it(collections);
    while (it.hasNext()) {
        if (it.next().value()->gameList().allGames().isEmpty()) {
            qWarning().noquote() << QObject::tr("No games found for collection `%1`, ignored")
                                                .arg(it.value()->tag());
            delete it.value();
            it.remove();
        }
    }
}

} // namespace


DataFinder::DataFinder(QObject* parent)
    : QObject(parent)
{
    m_providers.emplace_back(new providers::pegasus::PegasusProvider());
    m_providers.emplace_back(new providers::es2::Es2Provider());
#ifndef Q_PROCESSOR_ARM
    m_providers.emplace_back(new providers::steam::SteamProvider());
#endif

    for (auto& provider : m_providers) {
        connect(provider.get(), &providers::Provider::gameCountChanged,
                this, &DataFinder::totalCountChanged);
        connect(provider.get(), &providers::Provider::romDirFound,
                this, &DataFinder::onRomDirFound,
                Qt::DirectConnection);
    }
}

// Providers can add new games, new collections and further directories
// to check for metadata info.
void DataFinder::runListProviders(QHash<QString, Types::Game*>& games,
                                  QHash<QString, Types::Collection*>& collections)
{
    for (auto& provider : m_providers)
        provider->find(games, collections);

    static_cast<providers::pegasus::PegasusProvider*>(m_providers.front().get())
        ->find_in_dirs(m_thirdparty_rom_dirs, games, collections);

    removeEmptyCollections(collections);
}

void DataFinder::onRomDirFound(QString dir_path)
{
    const QFileInfo entry(dir_path);
    if (entry.exists() && entry.isDir())
        m_thirdparty_rom_dirs << dir_path;
}

void DataFinder::runMetadataProviders(const QHash<QString, Types::Game*>& games,
                                      const QHash<QString, Types::Collection*>& collections)
{
    emit metadataSearchStarted();

    m_thirdparty_rom_dirs.removeDuplicates();
    pegasus_assets::findAssets(m_thirdparty_rom_dirs, games);
    m_thirdparty_rom_dirs.clear(); // free memory

    for (auto& provider : qAsConst(m_providers))
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
