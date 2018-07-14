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


#include "StatsDB.h"

#include "LocaleUtils.h"
#include "Paths.h"
#include "modeldata/gaming/Collection.h"
#include "modeldata/gaming/Game.h"

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>


namespace {

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

    bool open() { return m_db.open(); }
    bool startTransaction() { return m_db.transaction(); }
    bool commit() { return m_db.commit(); }

    bool hasTable(const QString& table_name) {
        return m_db.tables().contains(table_name);
    }

private:
    QSqlDatabase m_db;
};

QString default_db_path()
{
    return paths::writableConfigDir() + QStringLiteral("/stats.db");
}

void print_query_error(const QSqlQuery& query)
{
    const auto error = query.lastError();
    if (error.isValid())
        qWarning().noquote() << error.text();
}

void on_create_table_fail(QSqlQuery& query)
{
    qWarning().noquote() << tr_log("Failed to create database tables");
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

int get_path_id(const modeldata::Game* const game)
{
    {
        QSqlQuery query;
        query.prepare(QStringLiteral("SELECT id FROM paths WHERE path = ?;"));
        query.addBindValue(game->fileinfo().canonicalFilePath());
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
        query.addBindValue(game->fileinfo().canonicalFilePath());
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

} // namespace


StatsWriter::StatsWriter(QObject* parent)
    : StatsWriter(default_db_path(), parent)
{}

StatsWriter::StatsWriter(QString db_path, QObject* parent)
    : QObject(parent)
    , m_db_path(std::move(db_path))
{}

void StatsWriter::storePlay(const modeldata::Collection* const collection,
                            const modeldata::Game* const game,
                            const QDateTime& start_time, const qint64 duration)
{
    Q_ASSERT(game);
    Q_ASSERT(collection);

    // Steam saves play time in its own files
    if (collection->name() == QLatin1String("Steam"))
        return;


    SqlDefaultConnection channel(m_db_path);
    if (!channel.open()) {
        qWarning().noquote() << tr_log("Could not open or create `%1`, play time will not be saved")
                                .arg(m_db_path);
        return;
    }


    channel.startTransaction();

    if (!create_missing_tables(channel))
        return;

    const int path_id = get_path_id(game);
    if (path_id == -1)
        return;

    save_play_entry(path_id, start_time, duration);
    channel.commit();
}

void StatsReader::readDB(const HashMap<QString, modeldata::GamePtr>& games,
                         const QString& db_path)
{
    const QString real_db_path = db_path.isEmpty() ? default_db_path() : db_path;
    if (!QFileInfo::exists(real_db_path)) {
        // Database does not exists yet
        return;
    }

    SqlDefaultConnection channel(real_db_path);
    if (!channel.open()) {
        qWarning().noquote() << tr_log("Could not open `%1`, play times will not be loaded")
                                .arg(real_db_path);
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
    while (query.next()) {
        const QString path = query.value(0).toString();
        if (!games.count(path))
            continue;

        const qint64 start_epoch = query.value(1).toLongLong();
        const qint64 duration = query.value(2).toLongLong();

        const modeldata::GamePtr& game = games.at(path);
        game->last_played = QDateTime::fromSecsSinceEpoch(start_epoch + duration);
        game->playtime += std::max(static_cast<qint64>(0), duration);
        game->playcount++;
    }
}
