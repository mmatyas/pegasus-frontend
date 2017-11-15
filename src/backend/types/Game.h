// Pegasus Frontend
// Copyright (C) 2017  Mátyás Mustoha
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

#include <QDateTime>
#include <QString>
#include <QObject>

namespace Types { class GameAssets; }


namespace Types {

class Game : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString title MEMBER m_title CONSTANT)
    Q_PROPERTY(QString description MEMBER m_description CONSTANT)
    Q_PROPERTY(QString developer MEMBER m_developer CONSTANT)
    Q_PROPERTY(QString publisher MEMBER m_publisher CONSTANT)
    Q_PROPERTY(QString genre MEMBER m_genre CONSTANT)
    Q_PROPERTY(int year MEMBER m_year CONSTANT)
    Q_PROPERTY(int month MEMBER m_month CONSTANT)
    Q_PROPERTY(int day MEMBER m_day CONSTANT)
    Q_PROPERTY(int players MEMBER m_players CONSTANT)
    Q_PROPERTY(bool favorite MEMBER m_favorite CONSTANT)
    Q_PROPERTY(float rating MEMBER m_rating CONSTANT)
    Q_PROPERTY(int playCount MEMBER m_playcount CONSTANT)
    Q_PROPERTY(QDateTime lastPlayed MEMBER m_lastplayed CONSTANT)
    Q_PROPERTY(Types::GameAssets* assets READ assets CONSTANT)

public:
    explicit Game(QString path, QObject* parent = nullptr);

    const QString m_rom_path;
    const QString m_rom_basename;

    QString m_title;
    QString m_description;
    QString m_developer;
    QString m_publisher;
    QString m_genre;

    int m_players;
    bool m_favorite;
    float m_rating;

    int m_year;
    int m_month;
    int m_day;

    int m_playcount;
    QDateTime m_lastplayed;

    QString m_launch_cmd;

    GameAssets* assets() const { return m_assets; }

private:
    GameAssets* m_assets;
};

} // namespace Types
