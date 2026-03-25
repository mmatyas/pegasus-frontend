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


#include "PlaytimeStats.h"

#include "Log.h"
#include "Paths.h"
#include "model/gaming/Game.h"
#include "model/gaming/GameFile.h"
#include "providers/SearchContext.h"
#include "utils/PathTools.h"
#include "utils/SqliteDb.h"

#include <QFileInfo>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QtConcurrent/QtConcurrent>


namespace {
QString default_db_path()
{
    return paths::writableConfigDir() + QStringLiteral("/stats.db");
}

void print_query_error(const QString& log_tag, const QSqlQuery& query)
{
    const auto error = query.lastError();
    if (error.isValid())
        Log::warning(log_tag, error.text());
}

void on_create_table_fail(const QString& log_tag, QSqlQuery& query)
{
    Log::warning(log_tag, LOGMSG("Failed to create database tables"));
    print_query_error(log_tag, query);
}

bool create_missing_tables(const QString& log_tag, SqliteDb& channel)
{
    if (!channel.hasTable(QStringLiteral("paths"))) {
        QSqlQuery query;
        query.prepare(QStringLiteral(
            "CREATE TABLE paths"
              "(" "id INTEGER PRIMARY KEY"
              "," "path TEXT UNIQUE NOT NULL"
            ");"
        ));
        if (!query.exec()) {
            on_create_table_fail(log_tag, query);
            return false;
        }
    }
    if (!channel.hasTable(QStringLiteral("plays"))) {
        QSqlQuery query;
        query.prepare(QStringLiteral(
            "CREATE TABLE plays"
              "(" "id INTEGER PRIMARY KEY"
              "," "path_id INTEGER NOT NULL REFERENCES plays(id)"
              "," "start_time INTEGER NOT NULL"
              "," "duration INTEGER NOT NULL"
            ");"
        ));
        if (!query.exec()) {
            on_create_table_fail(log_tag, query);
            return false;
        }
    }

    return true;
}

int get_path_id(const QString& log_tag, const QString& game_key, bool insert_on_missing = true)
{
    {
        QSqlQuery query;
        query.prepare(QStringLiteral("SELECT id FROM paths WHERE path = ?;"));
        query.addBindValue(game_key);
        if (!query.exec()) {
            print_query_error(log_tag, query);
            return -1;
        }
        if (query.next())
            return query.value(0).toInt();
    }
    // no hit -> insert
    if (insert_on_missing) {
        QSqlQuery query;
        query.prepare(QStringLiteral("INSERT INTO paths VALUES(null, ?);"));
        query.addBindValue(game_key);
        if (!query.exec()) {
            print_query_error(log_tag, query);
            return -1;
        }
    }
    if (insert_on_missing) {
        QSqlQuery query;
        query.prepare(QStringLiteral("SELECT last_insert_rowid() FROM paths;"));
        if (!query.exec()) {
            print_query_error(log_tag, query);
            return -1;
        }
        if (query.next())
            return query.value(0).toInt();
    }

    return -1;
}

void save_play_entry(const QString& log_tag, const int path_id, const QDateTime& start_time, const qint64 duration)
{
    Q_ASSERT(path_id != -1);
    Q_ASSERT(start_time.isValid());
    Q_ASSERT(0 <= duration);

    QSqlQuery query;
    query.prepare(QStringLiteral("INSERT INTO plays VALUES(null, ?, ?, ?);"));
    query.addBindValue(path_id);
    query.addBindValue(start_time.toSecsSinceEpoch());
    query.addBindValue(duration);
    if (!query.exec())
        print_query_error(log_tag, query);
}

void migrate_play_entry(const QString& log_tag, int old_path_id, int new_path_id)
{
    Q_ASSERT(old_path_id != -1);
    Q_ASSERT(new_path_id != -1);

    // update plays to use new path
    QSqlQuery update_plays_query;
    update_plays_query.prepare(QStringLiteral("UPDATE plays SET path_id = ? WHERE path_id = ?;"));
    update_plays_query.addBindValue(new_path_id);
    update_plays_query.addBindValue(old_path_id);
    if (!update_plays_query.exec()) {
        print_query_error(log_tag, update_plays_query);
        return;
    }

    // delete old path
    QSqlQuery delete_path_query;
    delete_path_query.prepare(QStringLiteral("DELETE FROM paths WHERE id = ?;"));
    delete_path_query.addBindValue(old_path_id);
    if (!delete_path_query.exec())
        print_query_error(log_tag, delete_path_query);
}

void update_modelgame(model::GameFile* const gamefile, const QDateTime& start_time, const qint64 duration)
{
    Q_ASSERT(gamefile);
    gamefile->update_playstats(1, duration, start_time.addSecs(duration));
}

} // namespace


namespace providers {
namespace playtime {

PlaytimeStats::PlaytimeStats(QObject* parent)
    : PlaytimeStats(default_db_path(), parent)
{}

PlaytimeStats::PlaytimeStats(QString db_path, QObject* parent)
    : Provider(QLatin1String("pegasus_playtime"), QStringLiteral("Pegasus Playtime"), PROVIDER_FLAG_INTERNAL | PROVIDER_FLAG_HIDE_PROGRESS, parent)
    , m_db_path(std::move(db_path))
{}

Provider& PlaytimeStats::run(SearchContext& sctx)
{
    if (!QFileInfo::exists(m_db_path))
        return *this;

    SqliteDb channel(m_db_path);
    if (!channel.open()) {
        Log::error(display_name(), LOGMSG("Could not open `%1`, play times will not be loaded")
            .arg(m_db_path));
        return *this;
    }
    // No entries yet
    if (!channel.hasTable(QStringLiteral("paths")) || !channel.hasTable(QStringLiteral("plays")))
        return *this;


    QSqlQuery query;
    query.prepare(QStringLiteral(
        "SELECT paths.path, plays.start_time, plays.duration"
        " FROM plays"
        " INNER JOIN paths ON plays.path_id=paths.id;"
    ));
    if (!query.exec()) {
        print_query_error(display_name(), query);
        return *this;
    }


    struct Stats {
        int playcount { 0 };
        qint64 playtime { 0 };
        QDateTime last_played;
    };
    HashMap<model::GameFile*, Stats> stat_map;

    QMutexLocker lock(&m_queue_guard);

    while (query.next()) {
        const QString path = query.value(0).toString();
        model::GameFile* game_ptr = sctx.gamefile_by_uri(path);
        if (!game_ptr) {
            game_ptr = sctx.gamefile_by_filepath(path);
            // schedule a data migration if path is being used for a URI game
            if (game_ptr && game_ptr->hasUri())
                m_pending_migrations.emplace_back(game_ptr);
        }
        if (!game_ptr)
            continue;

        const qint64 start_epoch = query.value(1).toLongLong();
        const qint64 duration = query.value(2).toLongLong();

        Stats& stats = stat_map[game_ptr];
        stats.last_played = QDateTime::fromSecsSinceEpoch(start_epoch + duration);
        stats.playtime += std::max(static_cast<qint64>(0), duration);
        stats.playcount++;
    }

    // trigger update only once
    // TODO: C++17
    for (const auto& pair : stat_map) {
        model::GameFile* const gamefile = pair.first;
        const Stats& stats = pair.second;
        gamefile->update_playstats(stats.playcount, stats.playtime, stats.last_played);
    }

    // process any migrations
    if (m_active_migrations.empty())
        start_processing();

    return *this;
}

void PlaytimeStats::onGameLaunched(model::GameFile* const gamefile)
{
    Q_ASSERT(gamefile);
    m_last_launch_time = QDateTime::currentDateTimeUtc();
}

void PlaytimeStats::onGameFinished(model::GameFile* const gamefile)
{
    Q_ASSERT(gamefile);
    Q_ASSERT(m_last_launch_time.isValid());

    QMutexLocker lock(&m_queue_guard);

    const auto now = QDateTime::currentDateTimeUtc();
    const auto duration = m_last_launch_time.secsTo(now);

    m_pending_tasks.emplace_back(
        gamefile,
        m_last_launch_time,
        duration
    );

    if (m_active_tasks.empty())
        start_processing();
}

void PlaytimeStats::start_processing()
{
    Q_ASSERT(m_active_tasks.empty() || m_active_migrations.empty());

    m_active_tasks.swap(m_pending_tasks);
    m_active_migrations.swap(m_pending_migrations);

    QtConcurrent::run([this]{
        emit startedWriting();

        while (!m_active_tasks.empty() || !m_active_migrations.empty()) {
            for (const QueueEntry& entry : m_active_tasks)
                update_modelgame(entry.gamefile, entry.launch_time, entry.duration);

            SqliteDb channel(m_db_path);
            if (!channel.open()) {
                Log::warning(display_name(), LOGMSG("Could not open or create `%1`, play time will not be saved")
                    .arg(m_db_path));
                break;
            }

            channel.startTransaction();

            if (!create_missing_tables(display_name(), channel)) {
                channel.rollback();
                break;
            }

            for (const QueueEntry& entry : m_active_tasks) {
                const QString path = entry.gamefile->hasUri()
                    ? entry.gamefile->uri()
                    : ::clean_abs_path(entry.gamefile->fileinfo());
                const int path_id = get_path_id(display_name(), path);
                if (path_id >= 0)
                    save_play_entry(display_name(), path_id, entry.launch_time, entry.duration);
            }

            for (const MigrationQueueEntry& entry : m_active_migrations) {
                // we don't have a uri to migrate??
                if (!entry.gamefile->hasUri())
                    continue;
                const QString uri = entry.gamefile->uri();

                const QString path = ::clean_abs_path(entry.gamefile->fileinfo());
                const int old_path_id = get_path_id(display_name(), path, false);
                // no path ID to migrate
                if (old_path_id == -1)
                    continue;

                const int new_path_id = get_path_id(display_name(), uri);
                if (new_path_id >= 0) {
                    Log::info(LOGMSG("Migrating playtime stats for '%1'.").arg(uri));
                    migrate_play_entry(display_name(), old_path_id, new_path_id);
                }
            }

            channel.commit();

            // pick up new tasks
            QMutexLocker lock(&m_queue_guard);
            m_active_tasks.clear();
            m_active_tasks.swap(m_pending_tasks);
            m_active_migrations.clear();
            m_active_migrations.swap(m_pending_migrations);
        }

        emit finishedWriting();
    });
}

} // namespace playtime
} // namespace providers
