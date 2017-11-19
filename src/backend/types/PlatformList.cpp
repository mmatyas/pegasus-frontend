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


#include "PlatformList.h"

#include "ListPropertyFn.h"

#include <QDebug>


namespace Types {

PlatformList::PlatformList(QObject* parent)
    : QObject(parent)
    , m_platform_idx(-1)
{
}

PlatformList::~PlatformList()
{
    for (auto& platform : qAsConst(m_platforms))
        delete platform;
}

Platform* PlatformList::current() const
{
    if (m_platform_idx < 0)
        return nullptr;

    Q_ASSERT(m_platform_idx < m_platforms.length());
    return m_platforms.at(m_platform_idx);
}

void PlatformList::setIndex(int idx)
{
    // Setting the index to a valid value causes changing the current platform
    // and the current game. Setting the index to an invalid value should not
    // change anything.

    if (idx == m_platform_idx)
        return;

    const bool valid_idx = (0 <= idx || idx < m_platforms.count());
    if (!valid_idx) {
        qWarning() << tr("Invalid platform index #%1").arg(idx);
        return;
    }

    m_platform_idx = idx;
    emit currentChanged();
    emit currentPlatformGameChanged();
}

QQmlListProperty<Platform> PlatformList::platformsProp()
{
    static const auto count = &listproperty_count<Platform>;
    static const auto at = &listproperty_at<Platform>;

    return {this, &m_platforms, count, at};
}

void PlatformList::onScanComplete()
{
    // NOTE: `tr` (see below) uses `int`; assuming we have
    //       less than 2 million games, it will be enough
    int game_count = 0;

    for (Platform* const platform : m_platforms) {
        connect(platform, &Platform::currentGameChanged,
                this, &PlatformList::currentPlatformGameChanged);

        Types::GameList& gamelist = platform->gameListMut();
        gamelist.lockGameList();
        game_count += gamelist.allGames().count();
    }
    qInfo().noquote() << tr("%n games found", "", game_count);

    if (!m_platforms.isEmpty())
        setIndex(0);
}

} // namespace Types
