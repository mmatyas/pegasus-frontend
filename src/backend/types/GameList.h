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

#include "Game.h"

#include <QObject>
#include <QQmlListProperty>
#include <QVector>

namespace Types { class Filters; }


namespace Types {

class GameList : public QObject {
    Q_OBJECT

    Q_PROPERTY(Types::Game* current
               READ current
               NOTIFY currentChanged)
    Q_PROPERTY(int index
               READ index
               WRITE setIndex
               NOTIFY currentChanged)
    Q_PROPERTY(QQmlListProperty<Types::Game> model
               READ getFilteredGamesProp
               NOTIFY filteredGamesChanged)
    Q_PROPERTY(QQmlListProperty<Types::Game> modelAll
               READ getAllGamesProp
               CONSTANT)

public:
    explicit GameList(QObject* parent = nullptr);
    ~GameList();

    Game* current() const;
    int index() const { return m_game_idx; }
    void setIndex(int);
    QQmlListProperty<Game> getFilteredGamesProp();
    QQmlListProperty<Game> getAllGamesProp();

public:
    // backend-side functions

    void clearFilters();
    void applyFilters(const Filters&);

    void addGame(QString path);
    void sortGames();
    void lockGameList();

    const QVector<Game*>& filteredGames() const { return m_filtered_games; }
    const QVector<Game*>& allGames() const { return m_all_games; }

signals:
    void currentChanged();
    void filteredGamesChanged();

private:
    int m_game_idx;

    QVector<Game*> m_all_games;
    QVector<Game*> m_filtered_games;

#ifdef QT_DEBUG
    bool m_gamelist_locked;
#endif
};

} // namespace Types
