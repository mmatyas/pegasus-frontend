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

#include "ListPropertyFn.h"
#include "LocaleUtils.h"
#include "Utils.h"
#include "model/Filters.h"
#include "modeldata/gaming/Collection.h"

#include <QDebug>


namespace {

bool game_property_is_true(const QVariant& property)
{
    switch (property.type()) {
        case QMetaType::QString:
            return !property.toString().isEmpty();
        case QMetaType::QStringList:
            return !property.toStringList().isEmpty();
        case QMetaType::QDate:
            return property.toDate().isValid();
        case QMetaType::QDateTime:
            return property.toDateTime().isValid();
        default:
            return property.toBool();
    }
}

bool game_property_contains(const QVariant& property, const QRegularExpression& regex)
{
    QString str;

    switch (property.type()) {
        case QMetaType::QStringList: {
            bool result = false;
            const QStringList list = property.toStringList();
            for (const QString& str : list)
                result |= regex.match(str).hasMatch();

            return result;
        }
        case QMetaType::QDate:
            str = property.toDate().toString(Qt::ISODate);
            break;
        case QMetaType::QDateTime:
            str = property.toDateTime().toString(Qt::ISODate);
            break;
        default:
            str = property.toString();
    }

    return regex.match(str).hasMatch();
}

bool game_property_equals(const QVariant& property, const QRegularExpression& regex)
{
    QString str;

    switch (property.type()) {
        case QMetaType::QStringList:
            return false; // disabled, TODO?
        case QMetaType::QDate:
            str = property.toDate().toString(Qt::ISODate);
            break;
        case QMetaType::QDateTime:
            str = property.toDateTime().toString(Qt::ISODate);
            break;
        default:
            str = property.toString();
    }

    const auto match = regex.match(str);
    return match.hasMatch() && match.capturedLength(0) == str.length();
}

bool filter_rule_matches(const model::Game& game, const model::FilterRule& rule)
{
    const QVariant property = game.property(rule.game_property.toLatin1().data());

    bool result = false;
    switch (rule.type) {
        case model::FilterRuleType::IS_TRUE:
        case model::FilterRuleType::IS_FALSE:
        case model::FilterRuleType::NOT_EMPTY:
        case model::FilterRuleType::EMPTY:
            result = game_property_is_true(property);
            break;
        case model::FilterRuleType::CONTAINS:
        case model::FilterRuleType::NOT_CONTAINS:
            result = game_property_contains(property, rule.regex);
            break;
        case model::FilterRuleType::EQUALS:
        case model::FilterRuleType::NOT_EQUALS:
            result = game_property_equals(property, rule.regex);
            break;
    }
    switch (rule.type) {
        case model::FilterRuleType::IS_FALSE:
        case model::FilterRuleType::EMPTY:
        case model::FilterRuleType::NOT_CONTAINS:
        case model::FilterRuleType::NOT_EQUALS:
            return !result;
        default:
            return result;
    }

}

bool filter_matches(const model::Game& game, const model::Filter& filter)
{
    for (const model::FilterRule& rule : filter.rules()) {
        if (!filter_rule_matches(game, rule))
            return false;
    }
    return true;
}

bool all_filters_match(const model::Game& game, const std::vector<model::Filter*>& filters)
{
    for (const model::Filter* const filter_ptr : filters) {
        Q_ASSERT(filter_ptr->enabled());
        if (!filter_matches(game, *filter_ptr))
            return false;
    }
    return true;
}

} // namespace


namespace model {

GameList::GameList(const std::vector<QSharedPointer<modeldata::Game>>& data, QObject* parent)
    : QObject(parent)
    , m_game_idx(-1)
{
    for (const QSharedPointer<modeldata::Game>& game_ptr : data) {
        m_all_games.append(new Game(game_ptr.data(), this));

        connect(m_all_games.last(), &Game::launchRequested,
                this, &GameList::gameLaunchRequested);
        connect(m_all_games.last(), &Game::favoriteChanged,
                this, &GameList::gameFavoriteChanged);
    }

    clearFilters();
}

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
        qWarning() << tr_log("Invalid game index #%1").arg(idx);
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

void GameList::applyFilters(const Filters& filters)
{
    // TODO: use QtConcurrent::blockingFilter

    std::vector<Filter*> enabled_filters;
    std::copy_if(filters.elements().cbegin(),
                 filters.elements().cend(),
                 std::back_inserter(enabled_filters),
                 [](Filter* filter){ return filter->enabled(); });

    QVector<Game*> filtered_games;
    for (Game* game_ptr :qAsConst(m_all_games)) {
        const Game& game = *game_ptr;

        const bool title_matches = game.title().contains(filters.m_game_title, Qt::CaseInsensitive);
        if (title_matches && all_filters_match(game, enabled_filters))
            filtered_games.append(game_ptr);
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

} // namespace model
