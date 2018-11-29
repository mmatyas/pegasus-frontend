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


#include "PlaytimeStats.h"

#include "LocaleUtils.h"
#include "Paths.h"
#include "model/gaming/Game.h"

#include <QDebug>
#include <QFileInfo>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QtConcurrent/QtConcurrent>


namespace {
static constexpr auto MSG_PREFIX = "Playtime:";

QString default_db_path()
{
    return paths::writableConfigDir() + QStringLiteral("/stats.db");
}

// Wrapper above Qt for auto-closing and freeing the connection
struct SqlDefaultConnection {
    explicit SqlDefaultConnection(const QString& db_path)
        : m_db(QSqlDatabase::addDatabase(QStringLiteral("QSQLITE")))
    {
        m_db.setDatabaseName(db_path);
    }
    ~SqlDefaultConnection()
    {
        if (!m_db.isOpen())
            return;

        m_db.rollback();

        const auto connection = m_db.connectionName();
        m_db = QSqlDatabase();
        QSqlDatabase::removeDatabase(connection);
    }
    SqlDefaultConnection(const SqlDefaultConnection&) = delete;
    SqlDefaultConnection& operator=(const SqlDefaultConnection&) = delete;

    bool open() { return m_db.open(); }
    bool startTransaction() { return m_db.transaction(); }
    bool commit() { return m_db.commit(); }

    bool hasTable(const QString& table_name) {
        return m_db.tables().contains(table_name);
    }

private:
    QSqlDatabase m_db;
};

void print_query_error(const QSqlQuery& query)
{
    const auto error = query.lastError();
    if (error.isValid())
        qWarning().noquote() << error.text();
}

void on_create_table_fail(QSqlQuery& query)
{
    qWarning().noquote() << MSG_PREFIX << tr_log("failed to create database tables");
    print_query_error(query);
}

bool create_missing_tables(SqlDefaultConnection& channel)
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
            on_create_table_fail(query);
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
            on_create_table_fail(query);
            return false;
        }
    }

    return true;
}

int get_path_id(const QString& game_key)
{
    {
        QSqlQuery query;
        query.prepare(QStringLiteral("SELECT id FROM paths WHERE path = ?;"));
        query.addBindValue(game_key);
        if (!query.exec()) {
            print_query_error(query);
            return -1;
        }
        if (query.next())
            return query.value(0).toInt();
    }
    // no hit -> insert
    {
        QSqlQuery query;
        query.prepare(QStringLiteral("INSERT INTO paths VALUES(null, ?);"));
        query.addBindValue(game_key);
        if (!query.exec()) {
            print_query_error(query);
            return -1;
        }
    }
    {
        QSqlQuery query;
        query.prepare(QStringLiteral("SELECT last_insert_rowid() FROM paths;"));
        if (!query.exec()) {
            print_query_error(query);
            return -1;
        }
        if (query.next())
            return query.value(0).toInt();
    }

    return -1;
}

void save_play_entry(const int path_id, const QDateTime& start_time, const qint64 duration)
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
        print_query_error(query);
}

void update_modelgame(model::Game* const game, const QDateTime& start_time, const qint64 duration)
{
    Q_ASSERT(game);

    game->updatePlayStats(duration, start_time.addSecs(duration));
}

} // namespace


namespace providers {
namespace playtime {

PlaytimeStats::PlaytimeStats(QObject* parent)
    : PlaytimeStats(default_db_path(), parent)
{}

PlaytimeStats::PlaytimeStats(QString db_path, QObject* parent)
    : Provider(parent)
    , m_db_path(std::move(db_path))
{}

void PlaytimeStats::findDynamicData(const QVector<model::Game*>&,
                                    const QVector<model::Collection*>&,
                                    const HashMap<QString, model::Game*>& modelgame_map)
{
    if (!QFileInfo::exists(m_db_path))
        return;

    SqlDefaultConnection channel(m_db_path);
    if (!channel.open()) {
        qWarning().noquote() << MSG_PREFIX
            << tr_log("Could not open `%1`, play times will not be loaded")
                      .arg(m_db_path);
        return;
    }
    // No entries yet
    if (!channel.hasTable(QStringLiteral("paths")) || !channel.hasTable(QStringLiteral("plays")))
        return;


    QSqlQuery query;
    query.prepare(QStringLiteral(
        "SELECT paths.path, plays.start_time, plays.duration"
        " FROM plays"
        " INNER JOIN paths ON plays.path_id=paths.id;"
    ));
    if (!query.exec()) {
        print_query_error(query);
        return;
    }

    struct Stats {
        int playcount { 0 };
        qint64 playtime { 0 };
        QDateTime last_played;
    };
    HashMap<QString, Stats> stat_map;

    while (query.next()) {
        const QString path = query.value(0).toString();
        if (!modelgame_map.count(path))
            continue;

        const qint64 start_epoch = query.value(1).toLongLong();
        const qint64 duration = query.value(2).toLongLong();

        Stats& stats = stat_map[path];
        stats.last_played = QDateTime::fromSecsSinceEpoch(start_epoch + duration);
        stats.playtime += std::max(static_cast<qint64>(0), duration);
        stats.playcount++;
    }
    // trigger update only once
    for (const auto& pair : stat_map) {
        model::Game* game = modelgame_map.at(pair.first);
        const Stats& stats = stat_map.at(pair.first);
        game->addPlayStats(stats.playcount, stats.playtime, stats.last_played);
    }
}

void PlaytimeStats::onGameLaunched(model::Game* const game)
{
    Q_ASSERT(game);
    m_last_launch_time = QDateTime::currentDateTimeUtc();
}

void PlaytimeStats::onGameFinished(model::Game* const game)
{
    Q_ASSERT(game);
    Q_ASSERT(m_last_launch_time.isValid());

    QMutexLocker lock(&m_queue_guard);

    const auto now = QDateTime::currentDateTimeUtc();
    const auto duration = m_last_launch_time.secsTo(now);

    m_pending_tasks.emplace_back(
        game,
        m_last_launch_time,
        duration
    );

    if (m_active_tasks.empty())
        start_processing();
}

void PlaytimeStats::start_processing()
{
    Q_ASSERT(m_active_tasks.empty());

    m_active_tasks.swap(m_pending_tasks);

    QtConcurrent::run([this]{
        emit startedWriting();

        while (!m_active_tasks.empty()) {
            SqlDefaultConnection channel(m_db_path);
            if (!channel.open()) {
                qWarning().noquote() << MSG_PREFIX
                    << tr_log("Could not open or create `%1`, play time will not be saved")
                              .arg(m_db_path);
                break;
            }

            channel.startTransaction();

            if (!create_missing_tables(channel))
                break;

            for (const QueueEntry& entry : m_active_tasks) {
                const int path_id = get_path_id(entry.game->data().fileinfo().canonicalFilePath());
                if (path_id == -1)
                    continue;

                save_play_entry(path_id, entry.launch_time, entry.duration);
                update_modelgame(entry.game, entry.launch_time, entry.duration);
            }

            channel.commit();
            m_active_tasks.clear();

            // pick up new tasks
            QMutexLocker lock(&m_queue_guard);
            m_active_tasks.swap(m_pending_tasks);
        }

        emit finishedWriting();
    });
}

} // namespace playtime
} // namespace providers
