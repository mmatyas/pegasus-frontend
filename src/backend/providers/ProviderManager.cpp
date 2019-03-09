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
#include "utils/StdHelpers.h"

#include "QtQmlTricks/QQmlObjectListModel.h"
#include <QDebug>
#include <QtConcurrent/QtConcurrent>
#include <unordered_set>


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
    const auto childs_end = ctx.collection_childs.cend();

    auto it = ctx.collections.begin();
    while (it != ctx.collections.end()) {
        const QString& key = it->first;
        const auto childs_it = ctx.collection_childs.find(key);

        const bool is_empty = childs_it == childs_end || childs_it->second.empty();
        if (is_empty) {
            qWarning().noquote()
                << tr_log("No games found for collection '%1', ignored").arg(key);

            ctx.collection_childs.erase(key);
            it = ctx.collections.erase(it);
            continue;
        }

        ++it;
    }
}

void remove_empty_games(HashMap<size_t, modeldata::Game>& games)
{
    auto it = games.begin();
    while (it != games.end()) {
        const modeldata::Game& game = it->second;
        if (game.files.empty()) {
            qWarning().noquote()
                << tr_log("No files defined for game '%1', ignored").arg(game.title);
            it = games.erase(it);
            continue;
        }

        ++it;
    }
}

void remove_duplicate_childs(providers::SearchContext& sctx)
{
    for (auto& entry : sctx.collection_childs) {
        std::vector<size_t>& child_list = entry.second;
        VEC_REMOVE_DUPLICATES(child_list);
    }
}

void remove_parentless_games(providers::SearchContext& sctx)
{
    // prepare
    std::unordered_set<size_t> parented_game_ids;
    size_t parented_game_cnt = 0; // TODO: map-reduce
    for (const auto& entry : sctx.collection_childs)
        parented_game_cnt += entry.second.size();

    parented_game_ids.reserve(parented_game_cnt);
    for (const auto& entry : sctx.collection_childs)
        parented_game_ids.insert(entry.second.cbegin(), entry.second.cend());

    // remove
    auto it = sctx.games.begin();
    while (it != sctx.games.end()) {
        const bool has_parent = parented_game_ids.count(it->first);
        if (!has_parent) {
            qWarning().noquote()
                << tr_log("Game '%1' does not belong to any collections, ignored").arg(it->second.title);
            it = sctx.games.erase(it);
            continue;
        }
        ++it;
    }
}

void run_list_providers(providers::SearchContext& ctx, const std::vector<ProviderPtr>& providers)
{
    for (const ProviderPtr& ptr : providers)
        ptr->findLists(ctx);

    remove_empty_collections(ctx);
    remove_empty_games(ctx.games);
    remove_duplicate_childs(ctx);
    remove_parentless_games(ctx);
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
    HashMap<size_t, model::Game*> q_game_map;
    q_game_map.reserve(ctx.games.size());

    for (auto& keyval : ctx.games) {
        auto q_game = new model::Game(std::move(keyval.second));
        q_game->moveToThread(ui_thread);
        q_game_map.emplace(keyval.first, q_game);
    }

    QVector<model::Game*> q_game_list;
    q_game_list.reserve(static_cast<int>(ctx.games.size()));
    for (auto& keyval : q_game_map)
        q_game_list.append(keyval.second);

    sort_games(q_game_list);
    game_model.append(q_game_list);


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
        const std::vector<size_t>& game_ids = ctx.collection_childs[q_coll->name()];

        QVector<model::Game*> q_childs;
        q_childs.reserve(static_cast<int>(game_ids.size()));

        for (size_t game_id : game_ids) {
            Q_ASSERT(q_game_map.count(game_id));
            q_childs.append(q_game_map.at(game_id));
        }

        sort_games(q_childs);
        q_coll->setGameList(q_childs);
    }


    path_map.reserve(ctx.path_to_gameid.size());
    for (const model::Game* const q_game : q_game_list) {
        for (model::GameFile* const q_gamefile : q_game->filesConst()) {
            QString path = q_gamefile->data().fileinfo.canonicalFilePath();
            Q_ASSERT(!path.isEmpty());
            if (Q_LIKELY(!path.isEmpty()))
                path_map.emplace(std::move(path), q_gamefile);
        }
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
