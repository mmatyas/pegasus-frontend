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


#include "GamepadManager.h"

#include <QGamepadManager>


namespace model {

GamepadManager::GamepadManager(QObject* parent)
    : QObject(parent)
{
    QObject::connect(QGamepadManager::instance(), &QGamepadManager::gamepadConnected,
                     this, &GamepadManager::bkOnConnected);
    QObject::connect(QGamepadManager::instance(), &QGamepadManager::gamepadDisconnected,
                     this, &GamepadManager::bkOnDisconnected);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
    QObject::connect(QGamepadManager::instance(), &QGamepadManager::gamepadNameChanged,
                     this, &GamepadManager::bkOnNameChanged);
#endif

    for (const int device_id : QGamepadManager::instance()->connectedGamepads())
        m_devices.append(new QGamepad(device_id, &m_devices));
}

void GamepadManager::bkOnConnected(int device_id)
{
    qDebug() << "connected" << device_id;
    m_devices.append(new QGamepad(device_id, &m_devices));
    emit connected(device_id);
}

void GamepadManager::bkOnDisconnected(int device_id)
{
    QString name;

    const auto it = std::find_if(
        m_devices.constBegin(),
        m_devices.constEnd(),
        [device_id](const QGamepad* const gp){ return gp->deviceId() == device_id; });
    if (it != m_devices.constEnd()) {
        name = (*it)->name();
        m_devices.remove(*it);
    }

    qDebug() << "disconnected" << device_id;
    emit disconnected(std::move(name));
}

void GamepadManager::bkOnNameChanged(int, QString)
{
}

} // namespace model

