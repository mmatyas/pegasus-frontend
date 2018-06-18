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


namespace model {

class Game : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString title READ title CONSTANT)
    Q_PROPERTY(QString summary READ summary CONSTANT)
    Q_PROPERTY(QString description READ description CONSTANT)

    Q_PROPERTY(QString developer READ developer CONSTANT)
    Q_PROPERTY(QString publisher READ publisher CONSTANT)
    Q_PROPERTY(QString genre READ genre CONSTANT)
    Q_PROPERTY(QStringList developerList READ developerList CONSTANT)
    Q_PROPERTY(QStringList publisherList READ publisherList CONSTANT)
    Q_PROPERTY(QStringList genreList READ genreList CONSTANT)

    Q_PROPERTY(QDate release READ release CONSTANT)
    Q_PROPERTY(int year READ year CONSTANT)
    Q_PROPERTY(int month READ month CONSTANT)
    Q_PROPERTY(int day READ day CONSTANT)

    Q_PROPERTY(int players READ players CONSTANT)
    Q_PROPERTY(bool favorite READ favorite WRITE setFavorite NOTIFY favoriteChanged)
    Q_PROPERTY(float rating READ rating CONSTANT)

    Q_PROPERTY(int playCount READ playCount NOTIFY playCountChanged)
    Q_PROPERTY(QDateTime lastPlayed READ lastPlayed NOTIFY lastPlayedChanged)

    Q_PROPERTY(model::GameAssets* assets READ assetsPtr CONSTANT)

public:
    explicit Game(modeldata::Game* const, QObject* parent = nullptr);

    const QString& title() const { return m_game->title; }

    Q_INVOKABLE void launch();

signals:
    void launchRequested(const modeldata::Game* const);
    void favoriteChanged();
    void playCountChanged();
    void lastPlayedChanged();

private:
    const QString& summary() const { return m_game->summary; }
    const QString& description() const { return m_game->description; }

    const QString& developer() const { return m_game->developer_str; }
    const QString& publisher() const { return m_game->publisher_str; }
    const QString& genre() const { return m_game->genre_str; }
    const QStringList& developerList() const { return m_game->developer_list; }
    const QStringList& publisherList() const { return m_game->publisher_list; }
    const QStringList& genreList() const { return m_game->genre_list; }

    const QDate& release() const { return m_game->release_date; }
    int year() const { return m_game->release_year; }
    int month() const { return m_game->release_month; }
    int day() const { return m_game->release_day; }

    int players() const { return m_game->player_count; }
    float rating() const { return m_game->rating; }
    bool favorite() const { return m_game->is_favorite; }
    void setFavorite(bool);

    int playCount() const { return m_game->playcount; }
    const QDateTime& lastPlayed() const { return m_game->lastplayed; }

    GameAssets* assetsPtr() { return &m_assets; }

private:
    modeldata::Game* const m_game;
    GameAssets m_assets;
};

} // namespace model
