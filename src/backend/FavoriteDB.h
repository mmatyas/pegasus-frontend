// Pegasus Frontend
// Copyright (C) 2018  Mátyás Mustoha
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

#include "types/gaming/CollectionList.h"

#include <QMutex>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QtConcurrent/QtConcurrent>


class FavoriteWriter : public QObject {
    Q_OBJECT

public:
    explicit FavoriteWriter(QObject* parent = nullptr);
    explicit FavoriteWriter(const QString& file, QObject* parent = nullptr);

    /// Prepares the list of favorites and starts writing it to the output
    /// as soon as there are no pending write operation.
    void queueTask(const model::CollectionList&);


signals:
    void startedWriting();
    void finishedWriting();

private:
    const QString m_db_path;
    QStringList m_pending_task;
    QStringList m_current_task;
    QMutex m_task_guard;
    QFutureWatcher<void> m_write_watcher;

    void start_processing();
};

class FavoriteReader {
public:
    /// Reads the list of favorites and marks the matching games as favorite.
    static void readDB(const QHash<QString, model::Game*>&, const QString& db_path = QString());
};
