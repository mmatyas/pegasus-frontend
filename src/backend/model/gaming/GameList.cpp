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

#include "LocaleUtils.h"
#include "model/ListPropertyFn.h"
#include "modeldata/gaming/CollectionData.h"
#include "utils/IndexShifter.h"

#include <QDebug>


namespace {

void sort_game_vec(QVector<model::Game*>& games)
{
    // remove duplicates
    std::sort(games.begin(), games.end());
    games.erase(std::unique(games.begin(), games.end()), games.end());

    // sort by name
    std::sort(games.begin(), games.end(),
        [](const model::Game* const a, const model::Game* const b) {
            return QString::localeAwareCompare(a->data().title, b->data().title) < 0;
        }
    );
}

} // namespace


namespace model {

GameList::GameList(QObject* parent)
    : QObject(parent)
    , m_game_idx(-1)
{}

void GameList::setModelData(QVector<Game*> games)
{
    Q_ASSERT(m_all_games.isEmpty());

    sort_game_vec(games);
    m_all_games = std::move(games);

    for (Game* const game : qAsConst(m_all_games)) {
        connect(game, &Game::launchRequested,
                this, &GameList::gameLaunchRequested);
        connect(game, &Game::favoriteChanged,
                this, &GameList::gameFavoriteChanged);
    }

    clearFilters();
}

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
        qWarning().noquote() << tr_log("Invalid game index #%1").arg(idx);
        return;
    }

    m_game_idx = idx;
    emit currentChanged();
}

void GameList::shiftIndex(IndexShiftDirection dir)
{
    if (m_filtered_games.isEmpty())
        return;

    const int target_idx = shifterFn(dir)(m_game_idx, m_filtered_games.count());
    setIndex(target_idx);
}

void GameList::incrementIndex() {
    shiftIndex(IndexShiftDirection::INCREMENT);
}

void GameList::decrementIndex() {
    shiftIndex(IndexShiftDirection::DECREMENT);
}

void GameList::incrementIndexNoWrap() {
    shiftIndex(IndexShiftDirection::INCREMENT_NOWRAP);
}

void GameList::decrementIndexNoWrap() {
    shiftIndex(IndexShiftDirection::DECREMENT_NOWRAP);
}

QQmlListProperty<Game> GameList::getFilteredGamesProp()
{
    static constexpr auto count = &listproperty_count<Game>;
    static constexpr auto at = &listproperty_at<Game>;

    return {this, &m_filtered_games, count, at};
}

QQmlListProperty<Game> GameList::getAllGamesProp()
{
    static constexpr auto count = &listproperty_count<Game>;
    static constexpr auto at = &listproperty_at<Game>;

    return {this, &m_all_games, count, at};
}

/// Selects the first game in the list of filtered games whose title starts with
/// the provided text's first character. If the text is empty or no such game exists,
/// nothing happens.
void GameList::jumpToLetter(const QString& text)
{
    if (text.isEmpty())
        return;

    const QChar query_char = text.at(0).toLower();
    // NOTE: while this could be optimized for performance,
    // the increase in memory usage might not worth it.
    for (int i = 0; i < m_filtered_games.count(); i++) {
        Q_ASSERT(!m_filtered_games[i]->title().isEmpty());
        if (m_filtered_games[i]->title().at(0).toLower() == query_char) {
            setIndex(i);
            return;
        }
    }
}

void GameList::clearFilters()
{
    if (m_all_games.isEmpty())
        return;

    m_filtered_games = m_all_games;
    Q_ASSERT(!m_filtered_games.isEmpty());
    emit filteredGamesChanged();

    // TODO: remember position before reset
    setIndex(0);
}

} // namespace model
