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

#include "GameAssets.h"
#include "modeldata/gaming/Collection.h"
#include "modeldata/gaming/Game.h"

#include <QObject>


#define DATA_CONST_PROP(type, api_name, member_name) \
    public: \
        const type& api_name() const { return m_game.member_name; } \
    private: \
        Q_PROPERTY(type api_name READ api_name CONSTANT)


namespace model {

class Game : public QObject {
    Q_OBJECT

    DATA_CONST_PROP(QString, title, title)
    DATA_CONST_PROP(QString, summary, summary)
    DATA_CONST_PROP(QString, description, description)

    Q_PROPERTY(QString developer READ developerString CONSTANT)
    Q_PROPERTY(QString publisher READ publisherString CONSTANT)
    Q_PROPERTY(QString genre READ genreString CONSTANT)
    DATA_CONST_PROP(QStringList, developerList, developers)
    DATA_CONST_PROP(QStringList, publisherList, publishers)
    DATA_CONST_PROP(QStringList, genreList, genres)

    Q_PROPERTY(int players READ players CONSTANT)
    Q_PROPERTY(float rating READ rating CONSTANT)
    DATA_CONST_PROP(QDate, release, release_date)

    Q_PROPERTY(int releaseYear READ releaseYear CONSTANT)
    Q_PROPERTY(int releaseMonth READ releaseMonth CONSTANT)
    Q_PROPERTY(int releaseDay READ releaseDay CONSTANT)

    Q_PROPERTY(bool favorite READ favorite WRITE setFavorite NOTIFY favoriteChanged)
    Q_PROPERTY(int playCount READ playCount NOTIFY playStatsChanged)
    Q_PROPERTY(int playTime READ playTime NOTIFY playStatsChanged)
    Q_PROPERTY(QDateTime lastPlayed READ lastPlayed NOTIFY playStatsChanged)

    Q_PROPERTY(model::GameAssets* assets READ assetsPtr CONSTANT)

    // deprecated
    Q_PROPERTY(int year READ releaseYear CONSTANT)
    Q_PROPERTY(int month READ releaseMonth CONSTANT)
    Q_PROPERTY(int day READ releaseDay CONSTANT)

public:
    explicit Game(modeldata::Game, QObject* parent = nullptr);

    Q_INVOKABLE void launch();

    const modeldata::Game& data() const { return m_game; }
    void setFavorite(bool);
    void addPlayStats(int playcount, qint64 playtime, QDateTime last_played);
    void updatePlayStats(qint64 duration, QDateTime time_finished);

signals:
    void launchRequested(const model::Game* const);

    void favoriteChanged();
    void playStatsChanged();

private:
    QString developerString() const { return joined_list(m_game.developers); }
    QString publisherString() const { return joined_list(m_game.publishers); }
    QString genreString() const { return joined_list(m_game.genres); }

    int players() const { return m_game.player_count; }
    float rating() const { return m_game.rating; }
    bool favorite() const { return m_game.is_favorite; }

    int releaseYear() const { return m_game.release_date.year(); }
    int releaseMonth() const { return m_game.release_date.month(); }
    int releaseDay() const { return m_game.release_date.day(); }


    int playCount() const { return m_game.playcount; }
    qint64 playTime() const { return m_game.playtime; }
    const QDateTime& lastPlayed() const { return m_game.last_played; }

    GameAssets* assetsPtr() { return &m_assets; }

private:
    modeldata::Game m_game;
    GameAssets m_assets;

    QString joined_list(const QStringList& list) const { return list.join(QLatin1String(", ")); }
};

} // namespace model
