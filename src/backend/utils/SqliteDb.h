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


#pragma once

#include "MoveOnly.h"

#include <QString>
#include <QSqlDatabase>


// Wrapper above Qt for auto-closing and freeing the connection
class SqliteDb {
public:
    explicit SqliteDb(const QString& db_path);
    ~SqliteDb();

    MOVE_ONLY(SqliteDb)

    bool open() { return m_db.open(); }
    bool startTransaction() { return m_db.transaction(); }
    bool commit() { return m_db.commit(); }

    bool hasTable(const QString& table_name);

private:
    QSqlDatabase m_db;
};
