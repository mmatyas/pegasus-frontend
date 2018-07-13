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
#include "modeldata/gaming/Game.h"

#include <QObject>


#define DATA_CONST_PROP(type, api_name, member_name) \
    public: \
        const type& api_name() const { return m_game->member_name; } \
    private: \
        Q_PROPERTY(type api_name READ api_name CONSTANT)


namespace model {

class Game : public QObject {
    Q_OBJECT

    DATA_CONST_PROP(QString, title, title)
    DATA_CONST_PROP(QString, summary, summary)
    DATA_CONST_PROP(QString, description, description)

    DATA_CONST_PROP(QString, developer, developer_str)
    DATA_CONST_PROP(QString, publisher, publisher_str)
    DATA_CONST_PROP(QString, genre, genre_str)
    DATA_CONST_PROP(QStringList, developerList, developer_list)
    DATA_CONST_PROP(QStringList, publisherList, publisher_list)
    DATA_CONST_PROP(QStringList, genreList, genre_list)

    DATA_CONST_PROP(QDate, release, release_date)
    DATA_CONST_PROP(int, year, release_year)
    DATA_CONST_PROP(int, month, release_month)
    DATA_CONST_PROP(int, day, release_day)

    DATA_CONST_PROP(int, players, player_count)
    DATA_CONST_PROP(float, rating, rating)

    Q_PROPERTY(bool favorite READ favorite WRITE setFavorite NOTIFY favoriteChanged)
    Q_PROPERTY(int playCount READ playCount NOTIFY playCountChanged)
    Q_PROPERTY(int playTime READ playTime NOTIFY playTimeChanged)
    Q_PROPERTY(QDateTime lastPlayed READ lastPlayed NOTIFY lastPlayedChanged)

    Q_PROPERTY(model::GameAssets* assets READ assetsPtr CONSTANT)

public:
    explicit Game(modeldata::Game* const, QObject* parent = nullptr);

    Q_INVOKABLE void launch();

signals:
    void launchRequested(const modeldata::Game* const);

    void favoriteChanged();
    void playCountChanged();
    void playTimeChanged();
    void lastPlayedChanged();

private:
    bool favorite() const { return m_game->is_favorite; }
    void setFavorite(bool);

    int playCount() const { return m_game->playcount; }
    qint64 playTime() const { return m_game->playtime; }
    const QDateTime& lastPlayed() const { return m_game->lastplayed; }

    GameAssets* assetsPtr() { return &m_assets; }

private:
    modeldata::Game* const m_game;
    GameAssets m_assets;
};

} // namespace model
