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
#include "modeldata/gaming/Collection.h"
#include "modeldata/gaming/Game.h"

#include <QFile>
#include <QMutexLocker>
#include <QTextStream>


namespace {

QString default_db_path()
{
    return paths::writableConfigDir() + QStringLiteral("/favorites.txt");
}

} // namespace


FavoriteWriter::FavoriteWriter(QObject* parent)
    : FavoriteWriter(default_db_path(), parent)
{}

FavoriteWriter::FavoriteWriter(const QString& file, QObject* parent)
    : QObject(parent)
    , m_db_path(file)
{
    connect(&m_write_watcher, &QFutureWatcher<void>::started,
            this, &FavoriteWriter::startedWriting);
    connect(&m_write_watcher, &QFutureWatcher<void>::finished,
            this, &FavoriteWriter::finishedWriting);
}

void FavoriteWriter::start_processing()
{
    m_current_task = m_pending_task;
    m_pending_task.clear();

    QFuture<void> future = QtConcurrent::run([this]{
        while (true) {
            QFile db_file(m_db_path);
            if (!db_file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                qWarning() << tr_log("Could not open `%1` for writing, favorites are not saved.")
                              .arg(m_db_path);
                return;
            }

            QTextStream db_stream(&db_file);
            for (const QString& fav : qAsConst(m_current_task))
                db_stream << fav << endl;

            QMutexLocker lock(&m_task_guard);
            m_current_task.clear();
            if (m_pending_task.isEmpty())
                return;

            m_current_task = m_pending_task;
            m_pending_task.clear();
        }
    });

    m_write_watcher.setFuture(future);
}

void FavoriteWriter::queueTask(const std::vector<modeldata::Collection>& coll_list)
{
    QMutexLocker lock(&m_task_guard);

    m_pending_task.clear();
    m_pending_task << QStringLiteral("# List of favorites, one path per line");
    for (const modeldata::Collection& coll : coll_list) {
        for (const modeldata::GamePtr& game : coll.games()) {
            if (game->is_favorite)
                m_pending_task << game->fileinfo().canonicalFilePath();
        }
    }

    if (m_current_task.isEmpty())
        start_processing();
}

void FavoriteReader::readDB(const HashMap<QString, modeldata::GamePtr>& games,
                            const QString& db_path)
{
    const QString real_db_path = db_path.isEmpty() ? default_db_path() : db_path;
    if (!QFileInfo::exists(real_db_path))
        return;

    QFile db_file(real_db_path);
    if (!db_file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << tr_log("Could not open `%1` for reading, favorites are not loaded.")
                      .arg(real_db_path);
        return;
    }

    QTextStream db_stream(&db_file);
    QString line;
    while (db_stream.readLineInto(&line)) {
        if (line.startsWith('#'))
            continue;

        if (games.count(line))
            games.at(line)->is_favorite = true;
    }
}
