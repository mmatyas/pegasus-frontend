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

#include "utils/FwdDeclModelData.h"
#include "utils/HashMap.h"

#include <QMutex>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QtConcurrent/QtConcurrent>


class StatsWriter : public QObject {
    Q_OBJECT

public:
    explicit StatsWriter(QObject* parent = nullptr);
    explicit StatsWriter(QString db_path, QObject* parent = nullptr);

    void storePlay(const modeldata::Collection* const,
                   const modeldata::Game* const,
                   const QDateTime& start_time,
                   const qint64 duration);

signals:
    void startedWriting();
    void finishedWriting();

private:
    const QString m_db_path;
};

class StatsReader {
public:
    static void readDB(const HashMap<QString, modeldata::GamePtr>&,
                       const QString& db_path = QString());
};
