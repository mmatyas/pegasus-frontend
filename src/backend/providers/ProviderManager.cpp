// Pegasus Frontend
// Copyright (C) 2017-2019  Mátyás Mustoha
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

void remove_empty_collections(providers::SearchContext& ctx)
{
    std::vector<QString> empty_colls;

    for (const auto& coll_entry : ctx.collections) {
        const auto it = ctx.collection_childs.find(coll_entry.first);
        if (it != ctx.collection_childs.cend() && ctx.collection_childs.at(it->first).size() > 0)
            continue;

        empty_colls.push_back(coll_entry.first);
    }

    for (const QString& coll : empty_colls) {
        qWarning().noquote() << tr_log("No games found for collection '%1', ignored").arg(coll);
        ctx.collections.erase(coll);
        ctx.collection_childs.erase(coll);
    }
}

void run_list_providers(providers::SearchContext& ctx, const std::vector<ProviderPtr>& providers)
{
    for (const ProviderPtr& ptr : providers)
        ptr->findLists(ctx);

    remove_empty_collections(ctx);
}

void run_asset_providers(providers::SearchContext& ctx, const std::vector<ProviderPtr>& providers)
{
    for (const auto& provider : providers)
        provider->findStaticData(ctx);
}

void build_ui_layer(providers::SearchContext& ctx,
                    QThread* const ui_thread,
                    QQmlObjectListModel<model::Collection>& collection_model,
                    QQmlObjectListModel<model::Game>& game_model,
                    HashMap<QString, model::GameFile*>& path_map)
{
    QVector<model::Game*> q_games;
    q_games.reserve(static_cast<int>(ctx.games.size()));

    for (modeldata::Game& game : ctx.games) {
        auto q_game = new model::Game(std::move(game));
        q_game->moveToThread(ui_thread);
        q_games.append(q_game);
    }


    QVector<model::Collection*> q_collections;
    q_collections.reserve(static_cast<int>(ctx.collections.size()));

    for (auto& keyval : ctx.collections) {
        auto q_coll = new model::Collection(std::move(keyval.second));
        q_coll->moveToThread(ui_thread);
        q_collections.append(q_coll);
    }

    sort_collections(q_collections);
    collection_model.append(q_collections);


    for (model::Collection* const q_coll : q_collections) {
        const std::vector<size_t>& game_indices = ctx.collection_childs[q_coll->name()];

        QVector<model::Game*> q_childs;
        q_childs.reserve(static_cast<int>(game_indices.size()));

        for (size_t game_idx : game_indices)
            q_childs.append(q_games.at(static_cast<int>(game_idx)));

        sort_games(q_childs);
        q_coll->setGameList(q_childs);
    }


    path_map.reserve(ctx.path_to_gameidx.size());
    for (auto entry : ctx.path_to_gameidx) {
        const model::Game* const q_game = q_games.at(static_cast<int>(entry.second));

        for (model::GameFile* const q_gamefile : q_game->filesConst()) {
            QString path = q_gamefile->data().fileinfo.canonicalFilePath();
            Q_ASSERT(!path.isEmpty());
            if (Q_LIKELY(!path.isEmpty()))
                path_map.emplace(std::move(path), q_gamefile);
        }
    }


    sort_games(q_games);
    game_model.append(q_games);
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
        providers::SearchContext ctx;

        QElapsedTimer timer;
        timer.start();


        run_list_providers(ctx, m_providers);
        emit firstPhaseComplete(timer.restart());

        run_asset_providers(ctx, m_providers);
        emit secondPhaseComplete(timer.restart());

        HashMap<QString, model::GameFile*> path_map;
        build_ui_layer(ctx, parent()->thread(), collection_model, game_model, path_map);
        emit staticDataReady();

        for (const auto& provider : m_providers)
            provider->findDynamicData(collection_model.asList(), game_model.asList(), path_map);
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

void ProviderManager::onGameLaunched(model::GameFile* const game)
{
    if (m_init_seq.isRunning())
        return;

    for (const auto& provider : m_providers)
        provider->onGameLaunched(game);
}

void ProviderManager::onGameFinished(model::GameFile* const game)
{
    if (m_init_seq.isRunning())
        return;

    for (const auto& provider : m_providers)
        provider->onGameFinished(game);
}
