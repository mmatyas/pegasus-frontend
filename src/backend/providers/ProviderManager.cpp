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
#include "LocaleUtils.h"
#include "model/gaming/Collection.h"
#include "model/gaming/Game.h"
#include "utils/HashMap.h"
#include "utils/StdHelpers.h"

#include <QDebug>
#include <QtConcurrent/QtConcurrent>
#include <unordered_set>

using ProviderPtr = providers::Provider*;


namespace {
std::vector<ProviderPtr> enabled_providers()
{
    std::vector<ProviderPtr> out;
    for (const auto& provider : AppSettings::providers) {
        if (provider->enabled())
            out.emplace_back(provider.get());
    }
    return out;
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

void remove_duplicate_childs(HashMap<QString, std::vector<size_t>>& coll_childs)
{
    for (auto& entry : coll_childs) {
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

void postprocess_list_results(providers::SearchContext& ctx)
{
    QElapsedTimer timer;
    timer.start();

    remove_empty_collections(ctx);
    remove_empty_games(ctx.games);
    remove_duplicate_childs(ctx.collection_childs);
    remove_parentless_games(ctx);
    VEC_REMOVE_DUPLICATES(ctx.game_root_dirs);

    qInfo().noquote() << tr_log("Game list post-processing took %1ms").arg(timer.elapsed());
}

void run_list_providers(providers::SearchContext& ctx, const std::vector<ProviderPtr>& providers)
{
    QElapsedTimer timer;
    timer.start();

    for (const ProviderPtr& ptr : providers) {
        if (!(ptr->flags() & providers::PROVIDES_GAMES))
            continue;

        ptr->findLists(ctx);
        qInfo().noquote() << tr_log("%1: finished game searching in %2ms")
            .arg(ptr->name(), QString::number(timer.restart()));
    }
}

void run_asset_providers(providers::SearchContext& ctx, const std::vector<ProviderPtr>& providers)
{
    QElapsedTimer timer;
    timer.start();
    for (const auto& provider : providers) {
        if (!(provider->flags() & providers::PROVIDES_ASSETS))
            continue;

        provider->findStaticData(ctx);
        qInfo().noquote() << tr_log("%1: finished asset searching in %2ms")
            .arg(provider->name(), QString::number(timer.restart()));
    }
}

HashMap<QString, model::GameFile*> build_path_map(const QVector<model::Game*>& games)
{
    // prepare
    size_t gamefile_cnt = 0; // TODO: map-reduce
    for (const model::Game* const q_game : games)
        gamefile_cnt += q_game->data().files.size();

    // build
    HashMap<QString, model::GameFile*> result;
    result.reserve(gamefile_cnt);
    for (const model::Game* const q_game : games) {
        for (model::GameFile* const q_gamefile : q_game->filesConst()) {
            // NOTE: canonical file path is empty for Android apps
            QString path = q_gamefile->data().fileinfo.canonicalFilePath();
            if (Q_LIKELY(!path.isEmpty()))
                result.emplace(std::move(path), q_gamefile);
        }
    }
    return result;
}
} // namespace


ProviderManager::ProviderManager(QObject* parent)
    : QObject(parent)
{
    for (const auto& provider : AppSettings::providers) {
        connect(provider.get(), &providers::Provider::gameCountChanged,
                this, &ProviderManager::gameCountChanged);
    }
}

void ProviderManager::startStaticSearch(providers::SearchContext& out_sctx)
{
    Q_ASSERT(!m_future.isRunning());

    m_future = QtConcurrent::run([this, &out_sctx]{
        providers::SearchContext ctx;

        QElapsedTimer timer;
        timer.start();

        const std::vector<ProviderPtr> providers = enabled_providers();
        for (const auto& provider : providers)
            provider->load();

        run_list_providers(ctx, providers);
        postprocess_list_results(ctx);
        emit firstPhaseComplete(timer.restart());

        run_asset_providers(ctx, providers);
        emit secondPhaseComplete(timer.restart());

        std::swap(ctx, out_sctx);
        emit staticDataReady();
    });
}

void ProviderManager::startDynamicSearch(const QVector<model::Game*>& games,
                                         const QVector<model::Collection*>& collections)
{
    Q_ASSERT(!m_future.isRunning());

    m_future = QtConcurrent::run([this, &games, &collections]{
        QElapsedTimer timer;
        timer.start();

        const HashMap<QString, model::GameFile*> path_map = build_path_map(games);
        for (const auto& provider : AppSettings::providers)
            provider->findDynamicData(collections, games, path_map);

        emit dynamicDataReady(timer.elapsed());
    });
}

void ProviderManager::onGameFavoriteChanged(const QVector<model::Game*>& all_games)
{
    if (m_future.isRunning())
        return;

    for (const auto& provider : AppSettings::providers)
        provider->onGameFavoriteChanged(all_games);
}

void ProviderManager::onGameLaunched(model::GameFile* const game)
{
    if (m_future.isRunning())
        return;

    for (const auto& provider : AppSettings::providers)
        provider->onGameLaunched(game);
}

void ProviderManager::onGameFinished(model::GameFile* const game)
{
    if (m_future.isRunning())
        return;

    for (const auto& provider : AppSettings::providers)
        provider->onGameFinished(game);
}
