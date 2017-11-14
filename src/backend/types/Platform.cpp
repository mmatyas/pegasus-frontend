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


#include "Platform.h"


namespace Types {

Platform::Platform(QString name, QStringList rom_dirs,
                   QStringList rom_filters, QString launch_cmd,
                   QObject* parent)
    : QObject(parent)
    , m_short_name(std::move(name))
    , m_rom_dirs(std::move(rom_dirs))
    , m_rom_filters(std::move(rom_filters))
    , m_launch_cmd(std::move(launch_cmd))
{
    Q_ASSERT(!m_short_name.isEmpty());
    Q_ASSERT(!m_rom_dirs.isEmpty());
    Q_ASSERT(!m_rom_filters.isEmpty());

    connect(&m_gamelist, &GameList::currentChanged,
            this, &Platform::currentGameChanged);
}

} // namespace Types
