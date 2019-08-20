// Pegasus Frontend
// Copyright (C) 2017-2019  Mátyás Mustoha
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


#include "GamepadManagerQt.h"

#include <QGamepadManager>


namespace model {

GamepadManagerQt::GamepadManagerQt(QObject* parent)
    : GamepadManagerBackend(parent)
{
    QObject::connect(QGamepadManager::instance(), &QGamepadManager::gamepadConnected,
                     this, &GamepadManagerBackend::connected);
    QObject::connect(QGamepadManager::instance(), &QGamepadManager::gamepadDisconnected,
                     this, &GamepadManagerBackend::disconnected);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
    QObject::connect(QGamepadManager::instance(), &QGamepadManager::gamepadNameChanged,
                     this, &GamepadManagerBackend::nameChanged);
#endif

    for (const int device_id : QGamepadManager::instance()->connectedGamepads())
        emit connected(device_id);
}

} // namespace model
