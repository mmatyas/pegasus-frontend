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


#include "Game.h"

#include <QFileInfo>
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
} // namespace


namespace Types {

Game::Game(QFileInfo fileinfo, QObject* parent)
    : QObject(parent)
    , m_fileinfo(std::move(fileinfo))
    , m_title(m_fileinfo.completeBaseName())
    , m_players(1)
    , m_favorite(false)
    , m_rating(0)
    , m_playcount(0)
    , m_year(0)
    , m_month(0)
    , m_day(0)
{
}

void Game::addDeveloper(const QString& dev)
{
    addToStrAndList(dev, m_developer_str, m_developer_list);
}

void Game::addPublisher(const QString& pub)
{
    addToStrAndList(pub, m_publisher_str, m_publisher_list);
}

void Game::addGenre(const QString& genre)
{
    addToStrAndList(genre, m_genre_str, m_genre_list);
}

void Game::setRelease(QDate date)
{
    if (!date.isValid())
        return;

    m_release = std::move(date);
    m_year = m_release.year();
    m_month = m_release.month();
    m_day = m_release.day();
}

void Game::launch()
{
    emit launchRequested(this);
}

} // namespace Types
