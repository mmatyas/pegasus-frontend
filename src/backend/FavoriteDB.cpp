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


#include "FavoriteDB.h"

#include "LocaleUtils.h"
#include "Paths.h"

#include <QFile>
#include <QTextStream>


namespace {

QString default_db_path()
{
    return paths::writableConfigDir() + QStringLiteral("/favorites.txt");
}

} // namespace


FavoriteDB::FavoriteDB(QObject* parent)
    : FavoriteDB(default_db_path(), parent)
{}

FavoriteDB::FavoriteDB(const QString& file, QObject* parent)
    : QObject(parent)
    , m_db_path(file)
{
    connect(&m_write_watcher, &QFutureWatcher<void>::started,
            this, &FavoriteDB::startedWriting);
    connect(&m_write_watcher, &QFutureWatcher<void>::finished,
            this, &FavoriteDB::finishedWriting);
}

void FavoriteDB::start_processing()
{
    QFuture<void> future = QtConcurrent::run([this]{
        while (true) {
            {
                std::unique_lock<std::mutex> lock(m_task_guard);
                if (m_pending_task.isEmpty()) {
                    m_current_task.clear();
                    return;
                }

                m_current_task = m_pending_task;
                m_pending_task.clear();
            }

            QFile db_file(m_db_path);
            if (!db_file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                qWarning() << tr_log("Could not open `%1` for writing, favorites are not saved.")
                              .arg(m_db_path);
                return;
            }

            QTextStream db_stream(&db_file);
            for (const QString& fav : qAsConst(m_current_task))
                db_stream << fav << endl;
        }
    });

    m_write_watcher.setFuture(future);
}

void FavoriteDB::queueTask(const types::CollectionList& coll_list)
{
    std::unique_lock<std::mutex> lock(m_task_guard);

    m_pending_task.clear();
    for (const types::Collection* const coll : coll_list.elements()) {
        for (const types::Game* const game : coll->gameList().allGames()) {
            if (game->m_favorite)
                m_pending_task << game->m_fileinfo.canonicalFilePath();
        }
    }

    if (m_current_task.isEmpty())
        start_processing();
}
