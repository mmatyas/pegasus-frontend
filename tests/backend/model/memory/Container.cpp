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


#include "Container.h"


Container::Container(QObject* parent)
    : QObject(parent)
{
    setup();
}

Container::Container(QString settings_dir, QObject* parent)
    : QObject(parent)
    , m_memory(std::move(settings_dir))
{
    setup();
}

void Container::setup()
{
    connect(&m_memory, &model::Memory::dataChanged,
            this, &Container::memoryChanged);
}
