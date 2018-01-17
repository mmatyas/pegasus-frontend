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


#include "Collection.h"


namespace Types {

Collection::Collection(QString tag, QObject* parent)
    : QObject(parent)
    , m_tag(std::move(tag))
{
    Q_ASSERT(!m_tag.isEmpty());

    connect(&m_gamelist, &GameList::currentChanged,
            this, &Collection::currentGameChanged);
    connect(&m_gamelist, &GameList::launchRequested,
            this, [this](const Game* game){ emit launchRequested(this, game); });
}

void Collection::setName(QString str)
{
    Q_ASSERT(!str.isEmpty());
    m_name = str;
}

void Collection::setCommonLaunchCmd(QString str)
{
    Q_ASSERT(!str.isEmpty());
    m_launch_cmd = str;
}

} // namespace Types
