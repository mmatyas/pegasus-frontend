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


#include "SqliteDb.h"

#include <QStringList>


SqliteDb::SqliteDb(const QString& db_path)
    : m_db(QSqlDatabase::addDatabase(QStringLiteral("QSQLITE")))
{
    m_db.setDatabaseName(db_path);
}

SqliteDb::~SqliteDb()
{
    if (!m_db.isOpen())
        return;

    m_db.rollback();

    const auto connection = m_db.connectionName();
    m_db = QSqlDatabase();
    QSqlDatabase::removeDatabase(connection);
}

bool SqliteDb::hasTable(const QString& table_name)
{
    return m_db.tables().contains(table_name);
}
