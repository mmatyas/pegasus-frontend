// Pegasus Frontend
// Copyright (C) 2017-2020  Mátyás Mustoha
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
#include "Log.h"
#include "Provider.h"
#include "SearchContext.h"

#include <QtConcurrent/QtConcurrent>

using ProviderPtr = providers::Provider*;


namespace {
std::vector<ProviderPtr> enabled_providers()
{
    std::vector<ProviderPtr> out;
    for (const auto& provider : AppSettings::providers()) {
        if (provider->enabled())
            out.emplace_back(provider.get());
    }
    return out;
}
} // namespace


ProviderManager::ProviderManager(QObject* parent)
    : QObject(parent)
    , m_progress_finished(0.f)
    , m_progress_provider_weight(1.f)
    , m_target_collection_list(nullptr)
    , m_target_game_list(nullptr)
{
    // TODO: Improve detection of receiving signals from already finished providers
    /*for (const auto& provider : AppSettings::providers()) {
        connect(provider.get(), &providers::Provider::progressChanged,
                this, &ProviderManager::onProviderProgressChanged);
    }*/
}

void ProviderManager::run(
    QVector<model::Collection*>& out_collections,
    QVector<model::Game*>& out_games)
{
    Q_ASSERT(!m_future.isRunning());

    m_target_collection_list = &out_collections;
    m_target_game_list = &out_games;


    m_future = QtConcurrent::run([this]{
        providers::SearchContext sctx;
        sctx.enable_network();


        QElapsedTimer run_timer;
        run_timer.start();

        const std::vector<ProviderPtr> providers = enabled_providers();
        m_progress_finished = 0.f;
        m_progress_provider_weight = 1.f / providers.size();

        for (size_t i = 0; i < providers.size(); i++) {
            providers::Provider& provider = *providers[i];

            m_progress_finished = i * m_progress_provider_weight;
            emit progressChanged(m_progress_finished, provider.display_name());

            QElapsedTimer provider_timer;
            provider_timer.start();

            provider.run(sctx);

            Log::info(provider.display_name(), tr_log("Finished searching in %1ms")
                .arg(QString::number(provider_timer.restart())));
        }
        m_progress_finished = 1.f;
        emit progressChanged(m_progress_finished, QString());


        if (sctx.has_pending_downloads()) {
            QElapsedTimer network_timer;
            network_timer.start();

            Log::info(tr_log("Waiting for online sources..."));

            QEventLoop loop;
            connect(&sctx, &providers::SearchContext::downloadCompleted,
                    &loop, [&loop, &sctx]{ if (!sctx.has_pending_downloads()) loop.quit(); });
            loop.exec();

            Log::info(tr_log("Waiting for online sources took %1ms").arg(network_timer.elapsed()));
        }


        QElapsedTimer finalize_timer;
        finalize_timer.start();

        // TODO: C++17
        QVector<model::Collection*> collections;
        QVector<model::Game*> games;
        std::tie(collections, games) = sctx.finalize(parent());

        std::swap(collections, *m_target_collection_list);
        std::swap(games, *m_target_game_list);

        Log::info(tr_log("Game list post-processing took %1ms").arg(finalize_timer.elapsed()));
        emit finished();
    });
}

void ProviderManager::onProviderProgressChanged(float /*percent*/)
{
    // TODO: Improve detection of receiving signals from already finished providers
    //Q_ASSERT(0.f <= percent && percent <= 1.f);
    //emit progressChanged(m_progress_finished + m_progress_provider_weight * percent);
}


void ProviderManager::onGameFavoriteChanged(const QVector<model::Game*>& all_games) const
{
    if (m_future.isRunning())
        return;

    for (const auto& provider : AppSettings::providers())
        provider->onGameFavoriteChanged(all_games);
}

void ProviderManager::onGameLaunched(model::GameFile* const game) const
{
    if (m_future.isRunning())
        return;

    for (const auto& provider : AppSettings::providers())
        provider->onGameLaunched(game);
}

void ProviderManager::onGameFinished(model::GameFile* const game) const
{
    if (m_future.isRunning())
        return;

    for (const auto& provider : AppSettings::providers())
        provider->onGameFinished(game);
}
