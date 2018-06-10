// Pegasus Frontend
// Copyright (C) 2017-2018  Mátyás Mustoha
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


#include "Game.h"

#include <QStringBuilder>


namespace {

void addToStrAndList(const QString& new_val, QString& str, QStringList& list)
{
    if (new_val.isEmpty())
        return;

    if (str.isEmpty())
        str = new_val;
    else
        str += QLatin1String(", ") % new_val;

    list.append(new_val);
}

void addListToStrAndList(const QStringList& new_vals, QString& str, QStringList& list)
{
    for (const QString& new_val : new_vals)
        addToStrAndList(new_val, str, list);
}

} // namespace


namespace modeldata {

Game::Game(QFileInfo fileinfo)
    : title(fileinfo.completeBaseName())
    , player_count(1)
    , is_favorite(false)
    , rating(0.f)
    , playcount(0)
    , release_year(0)
    , release_month(0)
    , release_day(0)
    , m_fileinfo(std::move(fileinfo))
{
}

void Game::addDeveloper(const QString& val)
{
    addToStrAndList(val, developer_str, developer_list);
}
void Game::addPublisher(const QString& val)
{
    addToStrAndList(val, publisher_str, publisher_list);
}
void Game::addGenre(const QString& val)
{
    addToStrAndList(val, genre_str, genre_list);
}

void Game::addDevelopers(const QStringList& list)
{
    addListToStrAndList(list, developer_str, developer_list);
}
void Game::addPublishers(const QStringList& list)
{
    addListToStrAndList(list, publisher_str, publisher_list);
}
void Game::addGenres(const QStringList& list)
{
    addListToStrAndList(list, genre_str, genre_list);
}

void Game::setReleaseDate(QDate date)
{
    if (!date.isValid())
        return;

    release_date = std::move(date);
    release_year = release_date.year();
    release_month = release_date.month();
    release_day = release_date.day();
}

} // namespace modeldata
