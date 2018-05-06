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

#include "LocaleUtils.h"

#include <QDebug>


namespace types {

Collection::Collection(QString name, QObject* parent)
    : QObject(parent)
    , m_name(std::move(name))
{
    Q_ASSERT(!m_name.isEmpty());

    connect(&m_gamelist, &GameList::currentChanged,
            this, &Collection::currentGameChanged);
    connect(&m_gamelist, &GameList::launchRequested,
            this, [this](const Game* game){ emit launchRequested(this, game); });
}

const QString& Collection::tag() const {
    qWarning() << tr_log("Warning: `collection.tag` is deprecated and will be removed in the future. Do not use it in themes.");
    return m_name;
}

void Collection::setShortName(QString str)
{
    Q_ASSERT(!str.isEmpty());
    m_short_name = str.toLower();
}

void Collection::setCommonLaunchCmd(QString str)
{
    Q_ASSERT(!str.isEmpty());
    m_launch_cmd = str;
}

} // namespace types
