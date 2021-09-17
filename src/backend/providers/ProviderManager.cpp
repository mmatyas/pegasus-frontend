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
    , m_progress_step(1.f)
    , m_target_collection_list(nullptr)
    , m_target_game_list(nullptr)
{
    for (const auto& provider : AppSettings::providers()) {
        connect(provider.get(), &providers::Provider::progressChanged,
                this, &ProviderManager::onProviderProgressChanged);
    }
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

        size_t progress_sections = providers.size();
        for (const ProviderPtr provider : providers) {
            if (provider->flags() & providers::PROVIDER_FLAG_HIDE_PROGRESS)
                progress_sections--;
        }
        m_progress_finished = 0.f;
        m_progress_step = 1.f / std::max<size_t>(progress_sections, 1);
        m_progress_stage = QString();

        for (size_t i = 0; i < providers.size(); i++) {
            providers::Provider& provider = *providers[i];
            m_progress_stage = provider.display_name();

            emit progressChanged(m_progress_finished, m_progress_stage);

            QElapsedTimer provider_timer;
            provider_timer.start();

            provider.run(sctx);

            Log::info(provider.display_name(), LOGMSG("Finished searching in %1ms")
                .arg(QString::number(provider_timer.restart())));

            const bool has_progress = !(provider.flags() & providers::PROVIDER_FLAG_HIDE_PROGRESS);
            if (has_progress)
                m_progress_finished += m_progress_step;
        }
        m_progress_finished = 1.f;
        m_progress_stage = QString();
        emit progressChanged(m_progress_finished, m_progress_stage);


        if (sctx.has_pending_downloads()) {
            QElapsedTimer network_timer;
            network_timer.start();

            Log::info(LOGMSG("Waiting for online sources..."));

            QEventLoop loop;
            connect(&sctx, &providers::SearchContext::downloadCompleted,
                    &loop, [&loop, &sctx]{ if (!sctx.has_pending_downloads()) loop.quit(); });
            loop.exec();

            Log::info(LOGMSG("Waiting for online sources took %1ms").arg(network_timer.elapsed()));
        }


        QElapsedTimer finalize_timer;
        finalize_timer.start();

        // TODO: C++17
        QVector<model::Collection*> collections;
        QVector<model::Game*> games;
        std::tie(collections, games) = sctx.finalize(parent());

        std::swap(collections, *m_target_collection_list);
        std::swap(games, *m_target_game_list);

        Log::info(LOGMSG("Game list post-processing took %1ms").arg(finalize_timer.elapsed()));
        emit finished();
    });
}

void ProviderManager::onProviderProgressChanged(float percent)
{
    const float safe_percent = qBound(0.f, percent, 1.f);
    emit progressChanged(m_progress_finished + m_progress_step * safe_percent, m_progress_stage);
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
