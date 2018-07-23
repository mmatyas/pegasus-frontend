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


#pragma once

#include "providers/Provider.h"

#include <QDateTime>
#include <QMutex>


namespace providers {
namespace playtime {

class PlaytimeStats : public Provider {
    Q_OBJECT

public:
    explicit PlaytimeStats(QObject* parent = nullptr);
    explicit PlaytimeStats(QString db_path, QObject* parent = nullptr);

    void findDynamicData(const QVector<model::Game*>&,
                         const QVector<model::Collection*>&,
                         const HashMap<QString, model::Game*>&) final;
    void onGameLaunched(const model::Collection* const,
                        const model::Game* const) final;
    void onGameFinished(const model::Collection* const,
                        const model::Game* const) final;

signals:
    void startedWriting();
    void finishedWriting();

private:
    const QString m_db_path;

    QDateTime m_last_launch_time;

    struct QueueEntry {
        model::Game* const game;
        const QDateTime launch_time;
        const qint64 duration;

        QueueEntry(model::Game* const game, QDateTime launch_time, qint64 duration)
            : game(std::move(game))
            , launch_time(std::move(launch_time))
            , duration(std::move(duration))
        {}
    };
    std::vector<QueueEntry> m_pending_tasks;
    std::vector<QueueEntry> m_active_tasks;
    QMutex m_queue_guard;

    void start_processing();
};

} // namespace playtime
} // namespace providers
