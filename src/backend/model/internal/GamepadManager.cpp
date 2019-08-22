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

#include "ScriptRunner.h"
#include "GamepadManagerQt.h"
#include "GamepadManagerSDL2.h"


namespace {
void on_gamepad_reconfig()
{
    ScriptRunner::run(ScriptEvent::CONFIG_CHANGED);
    ScriptRunner::run(ScriptEvent::CONTROLS_CHANGED);
}

QQmlObjectListModel<model::Gamepad>::const_iterator
find_by_deviceid(QQmlObjectListModel<model::Gamepad>& model, int device_id)
{
    return std::find_if(
        model.constBegin(),
        model.constEnd(),
        [device_id](const model::Gamepad* const gp){ return gp->deviceId() == device_id; });
}
} // namespace


namespace model {

GamepadManager::GamepadManager(QObject* parent)
    : QObject(parent)
    , m_backend(new GamepadManagerSDL2(this))
{
    connect(m_backend, &GamepadManagerBackend::connected,
            this, &GamepadManager::bkOnConnected);
    connect(m_backend, &GamepadManagerBackend::disconnected,
            this, &GamepadManager::bkOnDisconnected);
    connect(m_backend, &GamepadManagerBackend::nameChanged,
            this, &GamepadManager::bkOnNameChanged);
    connect(m_backend, &GamepadManagerBackend::configChanged, on_gamepad_reconfig);

#ifndef Q_OS_ANDROID
    connect(m_backend, &GamepadManagerBackend::buttonChanged,
            &padbuttonnav, &GamepadButtonNavigation::onButtonChanged);
    connect(m_backend, &GamepadManagerBackend::axisChanged,
            &padaxisnav, &GamepadAxisNavigation::onAxisEvent);

    connect(&padaxisnav, &GamepadAxisNavigation::buttonChanged,
            &padbuttonnav, &GamepadButtonNavigation::onButtonChanged);
#endif // Q_OS_ANDROID

    m_backend->start();
}

void GamepadManager::bkOnConnected(int device_id, QString name)
{
    if (name.isEmpty())
        name = QLatin1String("generic");

    m_devices.append(new Gamepad(device_id, name, &m_devices));
    qInfo() << "Gamepad: connected" << device_id << m_devices.last()->name();
    emit connected(device_id);
}

void GamepadManager::bkOnDisconnected(int device_id)
{
    QString name;

    const auto it = find_by_deviceid(m_devices, device_id);
    if (it != m_devices.constEnd()) {
        name = (*it)->name();
        m_devices.remove(*it);
    }

    qInfo() << "Gamepad: disconnected" << device_id << name;
    emit disconnected(std::move(name));
}

void GamepadManager::bkOnNameChanged(int device_id, QString name)
{
    const auto it = find_by_deviceid(m_devices, device_id);
    if (it != m_devices.constEnd())
        (*it)->setName(name);
}

} // namespace model

