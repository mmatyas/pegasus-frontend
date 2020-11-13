// Pegasus Frontend
// Copyright (C) 2017-2020  Mátyás Mustoha
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

#include "utils/MoveOnly.h"

#include <QDateTime>
#include <QFileInfo>
#include <QString>

namespace model { class Game; }


namespace model {
QString pretty_filename(const QFileInfo& fi);


struct GameFileData {
    explicit GameFileData(QFileInfo);
    explicit GameFileData(QFileInfo, QString);

    const QFileInfo fileinfo;
    QString name;

    // TODO: in the future...
    // QString summary;
    // QString description;
    // QString launch_cmd;
    // QString launch_workdir;

    bool operator==(const GameFileData&) const;

    struct PlayStats {
        QDateTime last_played;
        qint64 play_time = 0;
        int play_count = 0;
    } playstats;
};


class GameFile : public QObject {
    Q_OBJECT

public:
    const QString& name() const { return m_data.name; }
    GameFile& setName(QString val) { m_data.name = std::move(val); return *this; }
    QString path() const { return m_data.fileinfo.filePath(); }
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QString path READ path CONSTANT)

    int playCount() const { return m_data.playstats.play_count; }
    qint64 playTime() const { return m_data.playstats.play_time; }
    const QDateTime& lastPlayed() const { return m_data.playstats.last_played; }
    Q_PROPERTY(int playCount READ playCount NOTIFY playStatsChanged)
    Q_PROPERTY(int playTime READ playTime NOTIFY playStatsChanged)
    Q_PROPERTY(QDateTime lastPlayed READ lastPlayed NOTIFY playStatsChanged)

    const QFileInfo& fileinfo() const { return m_data.fileinfo; }

public:
    explicit GameFile(QFileInfo, model::Game&);

    model::Game* parentGame() const;

    Q_INVOKABLE void launch();

    void update_playstats(int playcount, qint64 playtime, QDateTime last_played);

signals:
    void launchRequested();
    void playStatsChanged();

private:
    GameFileData m_data;
};


bool sort_gamefiles(const model::GameFile* const, const model::GameFile* const);
} // namespace model
