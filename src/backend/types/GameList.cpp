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


#include "GameList.h"

#include "Filters.h"
#include "ListPropertyFn.h"

#include <QDebug>


namespace Types {

GameList::GameList(QObject* parent)
    : QObject(parent)
    , m_game_idx(-1)
#ifdef QT_DEBUG
    , m_gamelist_locked(false)
#endif
{}

GameList::~GameList() = default;

Game* GameList::current() const
{
    if (m_game_idx < 0)
        return nullptr;

    Q_ASSERT(m_game_idx < m_filtered_games.length());
    return m_filtered_games.at(m_game_idx);
}

void GameList::setIndex(int idx)
{
    if (idx == m_game_idx)
        return;

    const bool valid_idx = (idx == -1) || (0 <= idx && idx < m_filtered_games.count());
    if (!valid_idx) {
        qWarning() << tr("Invalid game index #%1").arg(idx);
        return;
    }

    m_game_idx = idx;
    emit currentChanged();
}

QQmlListProperty<Game> GameList::getFilteredGamesProp()
{
    static const auto count = &listproperty_count<Game>;
    static const auto at = &listproperty_at<Game>;

    return {this, &m_filtered_games, count, at};
}

QQmlListProperty<Game> GameList::getAllGamesProp()
{
    static const auto count = &listproperty_count<Game>;
    static const auto at = &listproperty_at<Game>;

    return {this, &m_all_games, count, at};
}

void GameList::addGame(QString path)
{
#ifdef QT_DEBUG
    Q_ASSERT(!m_gamelist_locked);
#endif
    m_all_games.append(new Game(std::move(path), this));
}

void GameList::sortGames()
{
    std::sort(m_all_games.begin(), m_all_games.end(),
        [](const Game* a, const Game* b) {
            return QString::localeAwareCompare(a->m_rom_basename, b->m_rom_basename) < 0;
        }
    );
}

void GameList::lockGameList()
{
#ifdef QT_DEBUG
    Q_ASSERT(!m_gamelist_locked);
    m_gamelist_locked = true;
#endif

    clearFilters();
    emit allCountChanged();
}

void GameList::clearFilters()
{
    m_filtered_games = m_all_games;
    Q_ASSERT(!m_filtered_games.isEmpty());
    emit filteredGamesChanged();

    // TODO: remember position before reset
    setIndex(0);
}

void GameList::applyFilters(const Filters& filters)
{
    // TODO: use QtConcurrent::blockingFilter

    QVector<Game*> filtered_games;
    for (Game* const game_ptr : qAsConst(m_all_games)) {
        const Game& game = *game_ptr;

        if (game.m_title.contains(filters.m_title, Qt::CaseInsensitive)
            && game.m_players >= filters.m_player_count
            && (!filters.m_favorite || game.m_favorite)) {
            filtered_games.append(game_ptr);
        }
    }

    // only save the change and emit an event if there was an actual change
    if (filtered_games == m_filtered_games)
        return;

    m_filtered_games = filtered_games;
    emit filteredGamesChanged();

    // TODO: remember position before reset
    if (!m_filtered_games.isEmpty())
        setIndex(0);
    else
        setIndex(-1);
}

} // namespace Types
