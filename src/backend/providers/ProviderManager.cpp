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
#include "EnabledProviders.h"
#include "LocaleUtils.h"
#include "model/gaming/Collection.h"
#include "model/gaming/Game.h"
#include "utils/HashMap.h"

#include "QtQmlTricks/QQmlObjectListModel.h"
#include <QDebug>
#include <QtConcurrent/QtConcurrent>


namespace {
void sort_games(QVector<model::Game*>& games)
{
    std::sort(games.begin(), games.end(),
        [](const model::Game* const a, const model::Game* const b) {
            return QString::localeAwareCompare(a->title(), b->title()) < 0;
        }
    );
}

void sort_collections(QVector<model::Collection*>& collections)
{
    std::sort(collections.begin(), collections.end(),
        [](const model::Collection* const a, const model::Collection* const b) {
            return QString::localeAwareCompare(a->name(), b->name()) < 0;
        }
    );
}

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

void build_ui_layer(QThread* const ui_thread,
                    HashMap<QString, modeldata::Game>& games,
                    HashMap<QString, modeldata::Collection>& collections,
                    HashMap<QString, std::vector<QString>>& collection_childs,
                    QQmlObjectListModel<model::Game>& game_model,
                    QQmlObjectListModel<model::Collection>& collection_model,
                    HashMap<QString, model::Game*>& gameid_to_q_game)
{
    QVector<model::Game*> q_games;
    q_games.reserve(static_cast<int>(games.size()));
    gameid_to_q_game.reserve(games.size());

    for (auto& keyval : games) {
        auto qobj = new model::Game(std::move(keyval.second));
        qobj->moveToThread(ui_thread);
        q_games.append(qobj);
        gameid_to_q_game.emplace(keyval.first, q_games.last());
    }
    sort_games(q_games);
    game_model.append(q_games);


    QVector<model::Collection*> q_collections;
    q_collections.reserve(static_cast<int>(collections.size()));

    for (auto& keyval : collections) {
        auto qobj = new model::Collection(std::move(keyval.second));
        qobj->moveToThread(ui_thread);
        q_collections.append(qobj);
    }
    sort_collections(q_collections);
    collection_model.append(q_collections);


    for (model::Collection* const q_coll : q_collections) {
        QVector<model::Game*> q_childs;

        const std::vector<QString>& game_keys = collection_childs[q_coll->name()];
        for (const QString& game_key : game_keys)
            q_childs.append(gameid_to_q_game.at(game_key));

        sort_games(q_childs);
        q_coll->setGameList(q_childs);
    }
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

void ProviderManager::startSearch(QQmlObjectListModel<model::Game>& game_model,
                                  QQmlObjectListModel<model::Collection>& collection_model)
{
    Q_ASSERT(!m_init_seq.isRunning());

    m_init_seq = QtConcurrent::run([this, &game_model, &collection_model]{
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


        HashMap<QString, model::Game*> gameid_to_q_game;

        build_ui_layer(parent()->thread(),
                       games, collections, collection_childs,
                       game_model, collection_model, gameid_to_q_game);
        emit staticDataReady();


        for (const auto& provider : m_providers)
            provider->findDynamicData(game_model.asList(), collection_model.asList(), gameid_to_q_game);
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

void ProviderManager::onGameLaunched(model::Game* const game)
{
    if (m_init_seq.isRunning())
        return;

    for (const auto& provider : m_providers)
        provider->onGameLaunched(game);
}

void ProviderManager::onGameFinished(model::Game* const game)
{
    if (m_init_seq.isRunning())
        return;

    for (const auto& provider : m_providers)
        provider->onGameFinished(game);
}
