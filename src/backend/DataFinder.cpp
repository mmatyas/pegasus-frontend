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

#include "LocaleUtils.h"
#include "configfiles/FavoriteDB.h"
#include "modeldata/gaming/Collection.h"
#include "providers/es2/Es2Provider.h"
#include "providers/pegasus/PegasusProvider.h"

#ifndef Q_PROCESSOR_ARM
#include "providers/steam/SteamProvider.h"
#endif

#include <QDebug>


namespace {

void removeEmptyCollections(std::unordered_map<QString, modeldata::Collection>& collections)
{
    auto it = collections.begin();
    while (it != collections.end()) {
        if (it->second.games().empty()) {
            qWarning().noquote() << tr_log("No games found for collection '%1', ignored").arg(it->second.name());
            it = collections.erase(it);
            continue;
        }
        ++it;
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
void DataFinder::runListProviders(std::unordered_map<QString, QSharedPointer<modeldata::Game>>& games,
                                  std::unordered_map<QString, modeldata::Collection>& collections)
{
    for (size_t i = 1; i < m_providers.size(); i++)
        m_providers[i]->find(games, collections);

    // run the Pegasus provider only after the third-party
    // directories have been found
    m_providers.front()->find(games, collections);

    removeEmptyCollections(collections);
}

void DataFinder::onRomDirFound(QString dir_path)
{
    static_cast<providers::pegasus::PegasusProvider*>(m_providers.front().get())
        ->add_game_dir(std::move(dir_path));
}

void DataFinder::runMetadataProviders(const std::unordered_map<QString, QSharedPointer<modeldata::Game>>& games,
                                      const std::unordered_map<QString, modeldata::Collection>& collections)
{
    emit metadataSearchStarted();

    for (auto& provider : qAsConst(m_providers))
        provider->enhance(games, collections);

    FavoriteReader::readDB(games);
}

std::vector<modeldata::Collection> DataFinder::find()
{
    std::unordered_map<QString, modeldata::Collection> collections;
    std::unordered_map<QString, QSharedPointer<modeldata::Game>> games;

    runListProviders(games, collections);
    runMetadataProviders(games, collections);

    std::vector<modeldata::Collection> result;
    for (auto it = collections.begin(); it != collections.end(); ++it) {
        it->second.sortGames();
        result.emplace_back(std::move(it->second));
    }
    std::sort(result.begin(), result.end(),
        [](const modeldata::Collection& a, const modeldata::Collection& b) {
            return QString::localeAwareCompare(a.name(), b.name()) < 0;
        }
    );

    return result;
}
