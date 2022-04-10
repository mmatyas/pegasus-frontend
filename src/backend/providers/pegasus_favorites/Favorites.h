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


#pragma once

#include "providers/Provider.h"

#include <QMutex>


namespace providers {
namespace favorites {

class Favorites : public Provider {
    Q_OBJECT

public:
    explicit Favorites(QString db_path, QObject* parent = nullptr);
    explicit Favorites(QObject* parent = nullptr);

    Provider& run(SearchContext&) final;

    void onGameFavoriteChanged(const std::vector<model::Game*>&) final;

signals:
    void startedWriting();
    void finishedWriting();

private:
    const QString m_db_path;

    QStringList m_pending_task;
    QStringList m_active_task;
    QMutex m_task_guard;

    void start_processing();
};

} // namespace favorites
} // namespace providers
