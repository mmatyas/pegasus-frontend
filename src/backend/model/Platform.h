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

#include <QString>
#include <QQmlListProperty>

namespace ApiParts { class Filters; }


// NOTE: `QQmlListProperty` requires including the namespace in
// the template parameter! See QTBUG-15459.

namespace Model {

class Platform : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString shortName MEMBER m_short_name CONSTANT)
    Q_PROPERTY(QString longName MEMBER m_long_name CONSTANT)

    Q_PROPERTY(int currentGameIndex
               READ currentGameIndex WRITE setCurrentGameIndex
               RESET resetGameIndex
               NOTIFY currentGameIndexChanged)
    Q_PROPERTY(Model::Game* currentGame
               READ currentGame
               NOTIFY currentGameChanged)
    Q_PROPERTY(QQmlListProperty<Model::Game> games
               READ getFilteredGamesProp
               NOTIFY filteredGamesChanged)
    Q_PROPERTY(QQmlListProperty<Model::Game> allGames
               READ getAllGamesProp CONSTANT)

public:
    explicit Platform(QString name, QString rom_dir_path,
                      QStringList rom_filters, QString launch_cmd,
                      QObject* parent = nullptr);

    Model::Game* currentGame() const { return m_current_game; }
    int currentGameIndex() const { return m_current_game_idx; }
    void setCurrentGameIndex(int);
    void resetGameIndex();

    const QList<Game*>& games() const { return m_filtered_games; }
    const QList<Game*>& allGames() const { return m_all_games; }
    void addGame(QString path);
    void sortGames();
    void lockGameList();

    QQmlListProperty<Model::Game> getFilteredGamesProp();
    QQmlListProperty<Model::Game> getAllGamesProp();

    void clearFilters();
    void applyFilters(const ApiParts::Filters&);

    const QString m_short_name;
    const QString m_long_name;
    const QString m_rom_dir_path;
    const QStringList m_rom_filters;
    const QString m_launch_cmd;

signals:
    void currentGameIndexChanged();
    void currentGameChanged();
    void filteredGamesChanged();

private:
    int m_current_game_idx;
    Model::Game* m_current_game;

    QList<Game*> m_all_games;
    QList<Game*> m_filtered_games;

#ifdef QT_DEBUG
    bool m_gamelist_locked;
#endif
};

} // namespace Model
