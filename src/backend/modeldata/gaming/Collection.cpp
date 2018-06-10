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


#include "Collection.h"

#include "Game.h"


namespace modeldata {

Collection::Collection(QString name)
    : m_name(std::move(name))
{}

void Collection::setShortName(const QString& str)
{
    Q_ASSERT(!str.isEmpty());
    m_short_name = str.toLower();
}

void Collection::setLaunchCmd(QString str)
{
    Q_ASSERT(!str.isEmpty());
    m_launch_cmd = std::move(str);
}

void Collection::sortGames()
{
    // remove duplicates
    std::sort(m_games.begin(), m_games.end());
    m_games.erase(std::unique(m_games.begin(), m_games.end()), m_games.end());

    // sort by name
    std::sort(m_games.begin(), m_games.end(),
        [](const GamePtr& a, const GamePtr& b) {
            return QString::localeAwareCompare(a->fileinfo().completeBaseName(),
                                               b->fileinfo().completeBaseName()) < 0;
        }
    );
}

} // namespace modeldata
