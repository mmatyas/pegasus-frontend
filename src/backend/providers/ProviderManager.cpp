// Pegasus Frontend
// Copyright (C) 2017-2018  Mátyás Mustoha
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


#include "ProviderManager.h"

#include "AppSettings.h"
#include "LocaleUtils.h"
#include "model/gaming/Collection.h"
#include "model/gaming/Game.h"
#include "providers/pegasus/PegasusProvider.h"
#include "providers/pegasus_favorites/Favorites.h"
#include "providers/pegasus_playtime/PlaytimeStats.h"
#include "utils/HashMap.h"

#ifdef WITH_COMPAT_ES2
  #include "providers/es2/Es2Provider.h"
#endif
#ifdef WITH_COMPAT_STEAM
  #include "providers/steam/SteamProvider.h"
#endif
#ifdef WITH_COMPAT_GOG
  #include "providers/gog/GogProvider.h"
#endif
#ifdef WITH_COMPAT_ANDROIDAPPS
  #include "providers/android_apps/AndroidAppsProvider.h"
#endif
#ifdef WITH_COMPAT_SKRAPER
  #include "providers/skraper/SkraperAssetsProvider.h"
#endif

#include <QDebug>
#include <QtConcurrent/QtConcurrent>


namespace {

void remove_empty_collections(HashMap<QString, modeldata::Collection>& collections,
                              HashMap<QString, std::vector<QString>>& collection_childs)
{
    std::vector<QString> empty_colls;

    for (const auto& coll_entry : collections) {
        const auto it = collection_childs.find(coll_entry.first);
        if (it != collection_childs.cend() && collection_childs.at(it->first).size() > 0)
            continue;

        empty_colls.push_back(coll_entry.first);
    }

    for (const QString& coll : empty_colls) {
        qWarning().noquote() << tr_log("No games found for collection '%1', ignored").arg(coll);
        collections.erase(coll);
        collection_childs.erase(coll);
    }
}

void run_list_providers(const std::vector<ProviderPtr>& providers,
                        HashMap<QString, modeldata::Game>& games,
                        HashMap<QString, modeldata::Collection>& collections,
                        HashMap<QString, std::vector<QString>>& collection_childs)
{
    // run the providers in reverse, so higher priority providers can overwrite
    // the previous data. The Pegasus provider will run last, when the other
    // providers have already collected the extra dirs to check.
    for (auto it = providers.crbegin(); it != providers.crend(); ++it)
        (*it)->findLists(games, collections, collection_childs);

    remove_empty_collections(collections, collection_childs);
}

void build_ui_layer(HashMap<QString, modeldata::Game>& games,
                    HashMap<QString, modeldata::Collection>& collections,
                    HashMap<QString, std::vector<QString>>& collection_childs,
                    QVector<model::Game*>& game_vec,
                    QVector<model::Collection*>& collection_vec,
                    HashMap<QString, model::Game*>& modelgame_map)
{
    collection_vec.reserve(static_cast<int>(collections.size()));
    game_vec.reserve(static_cast<int>(games.size()));
    modelgame_map.reserve(games.size());

    for (auto& keyval : games) {
        game_vec.append(new model::Game(std::move(keyval.second)));
        modelgame_map.emplace(keyval.first, game_vec.last());
    }

    for (auto& keyval : collections)
        collection_vec.append(new model::Collection(std::move(keyval.second)));

    for (model::Collection* const coll : collection_vec) {
        QVector<model::Game*> childs;

        const std::vector<QString>& game_keys = collection_childs[coll->data().name()];
        for (const QString& game_key : game_keys)
            childs.append(modelgame_map.at(game_key));

        coll->setGameList(childs);
    }
}

void move_qobjs_to_thread(const QVector<model::Game*>& games,
                          const QVector<model::Collection*>& collections,
                          QThread* const target_thread)
{
    for (model::Collection* const coll : collections)
        coll->moveToThread(target_thread);

    for (model::Game* const game : games)
        game->moveToThread(target_thread);
}

} // namespace


ProviderManager::ProviderManager(QObject* parent)
    : QObject(parent)
{
    m_providers.emplace_back(new providers::pegasus::PegasusProvider());
    m_providers.emplace_back(new providers::favorites::Favorites());
    m_providers.emplace_back(new providers::playtime::PlaytimeStats());
#ifdef WITH_COMPAT_STEAM
    if (AppSettings::ext_providers.at(ExtProvider::STEAM).enabled)
        m_providers.emplace_back(new providers::steam::SteamProvider());
#endif
#ifdef WITH_COMPAT_GOG
    if (AppSettings::ext_providers.at(ExtProvider::GOG).enabled)
        m_providers.emplace_back(new providers::gog::GogProvider());
#endif
#ifdef WITH_COMPAT_ES2
    if (AppSettings::ext_providers.at(ExtProvider::ES2).enabled)
        m_providers.emplace_back(new providers::es2::Es2Provider());
#endif
#ifdef WITH_COMPAT_ANDROIDAPPS
    if (AppSettings::ext_providers.at(ExtProvider::ANDROIDAPPS).enabled)
        m_providers.emplace_back(new providers::android::AndroidAppsProvider());
#endif
#ifdef WITH_COMPAT_SKRAPER
    if (AppSettings::ext_providers.at(ExtProvider::SKRAPER).enabled)
        m_providers.emplace_back(new providers::skraper::SkraperAssetsProvider());
#endif

    for (const auto& provider : m_providers) {
        connect(provider.get(), &providers::Provider::gameCountChanged,
                this, &ProviderManager::gameCountChanged);
    }
}

void ProviderManager::startSearch()
{
    m_init_seq = QtConcurrent::run([this]{
        HashMap<QString, modeldata::Game> games;
        HashMap<QString, modeldata::Collection> collections;
        HashMap<QString, std::vector<QString>> collection_childs;

        QElapsedTimer timer;
        timer.start();


        run_list_providers(m_providers, games, collections, collection_childs);
        emit firstPhaseComplete(timer.restart());

        for (const auto& provider : m_providers)
            provider->findStaticData(games, collections, collection_childs);
        emit secondPhaseComplete(timer.restart());


        QVector<model::Collection*> collection_vec;
        QVector<model::Game*> game_vec;
        HashMap<QString, model::Game*> modelgame_map;

        build_ui_layer(games, collections, collection_childs,
                       game_vec, collection_vec, modelgame_map);
        move_qobjs_to_thread(game_vec, collection_vec, parent()->thread());
        emit staticDataReady(collection_vec, game_vec);


        for (const auto& provider : m_providers)
            provider->findDynamicData(game_vec, collection_vec, modelgame_map);
        emit thirdPhaseComplete(timer.elapsed());
    });
}

void ProviderManager::onGameFavoriteChanged(const QVector<model::Game*>& all_games)
{
    if (m_init_seq.isRunning())
        return;

    for (const auto& provider : m_providers)
        provider->onGameFavoriteChanged(all_games);
}

void ProviderManager::onGameLaunched(model::Collection* const collection, model::Game* const game)
{
    if (m_init_seq.isRunning())
        return;

    for (const auto& provider : m_providers)
        provider->onGameLaunched(collection, game);
}

void ProviderManager::onGameFinished(model::Collection* const collection, model::Game* const game)
{
    if (m_init_seq.isRunning())
        return;

    for (const auto& provider : m_providers)
        provider->onGameFinished(collection, game);
}
