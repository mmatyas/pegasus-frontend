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

#include "modeldata/gaming/GameData.h"

#include <QObject>
#include <QString>


namespace model {
class GameFile : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QString path READ path CONSTANT)
    Q_PROPERTY(int playCount READ playCount NOTIFY playStatsChanged)
    Q_PROPERTY(int playTime READ playTime NOTIFY playStatsChanged)
    Q_PROPERTY(QDateTime lastPlayed READ lastPlayed NOTIFY playStatsChanged)

public:
    explicit GameFile(modeldata::GameFile, QObject*);

    Q_INVOKABLE void launch();

    const modeldata::GameFile& data() const { return m_data; }

public:
    const QString& name() const { return m_data.name; }
    QString path() const { return m_data.fileinfo.filePath(); }
    int playCount() const { return m_data.play_count; }
    qint64 playTime() const { return m_data.play_time; }
    QDateTime lastPlayed() const { return m_data.last_played; }

public:
    void addPlayStats(int playcount, qint64 playtime, const QDateTime& last_played);
    void updatePlayTime(qint64 duration, QDateTime time_finished);

signals:
    void launchRequested();
    void playStatsChanged();

private:
    modeldata::GameFile m_data;
};
} // namespace model
