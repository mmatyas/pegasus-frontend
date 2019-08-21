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


namespace {
void on_gamepad_reconfig()
{
    ScriptRunner::run(ScriptEvent::CONFIG_CHANGED);
    ScriptRunner::run(ScriptEvent::CONTROLS_CHANGED);
}
} // namespace


namespace model {

GamepadManager::GamepadManager(QObject* parent)
    : QObject(parent)
    , m_backend(new GamepadManagerQt(this))
{
    connect(m_backend, &GamepadManagerBackend::connected,
            this, &GamepadManager::bkOnConnected);
    connect(m_backend, &GamepadManagerBackend::disconnected,
            this, &GamepadManager::bkOnDisconnected);
    connect(m_backend, &GamepadManagerBackend::nameChanged,
            this, &GamepadManager::bkOnNameChanged);
    connect(m_backend, &GamepadManagerBackend::configChanged, on_gamepad_reconfig);

#ifdef Q_OS_ANDROID
    #define SET_GAMEPAD_KEY(fnName, enumName) \
        padkeynav.setButton ## fnName ## Key(static_cast<Qt::Key>(GamepadKeyId::enumName));
    SET_GAMEPAD_KEY(A, A);
    SET_GAMEPAD_KEY(B, B);
    SET_GAMEPAD_KEY(X, X);
    SET_GAMEPAD_KEY(Y, Y);
    SET_GAMEPAD_KEY(L1, L1);
    SET_GAMEPAD_KEY(L2, L2);
    SET_GAMEPAD_KEY(L3, L3);
    SET_GAMEPAD_KEY(R1, R1);
    SET_GAMEPAD_KEY(R2, R2);
    SET_GAMEPAD_KEY(R3, R3);
    SET_GAMEPAD_KEY(Select, SELECT);
    SET_GAMEPAD_KEY(Start, START);
    SET_GAMEPAD_KEY(Guide, GUIDE);
    #undef SET_GAMEPAD_KEY
#else
    connect(m_backend, &GamepadManagerBackend::buttonChanged,
            &padbuttonnav, &GamepadButtonNavigation::onButtonChanged);
    connect(m_backend, &GamepadManagerBackend::axisChanged,
            &padaxisnav, &GamepadAxisNavigation::onAxisEvent);

    connect(&padaxisnav, &GamepadAxisNavigation::buttonChanged,
            &padbuttonnav, &GamepadButtonNavigation::onButtonChanged);
#endif // Q_OS_ANDROID
}

void GamepadManager::bkOnConnected(int device_id)
{
    m_devices.append(new QGamepad(device_id, &m_devices));

    qInfo() << "Gamepad: connected" << device_id << m_devices.last()->name();
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

    qInfo() << "Gamepad: disconnected" << device_id << name;
    emit disconnected(std::move(name));
}

void GamepadManager::bkOnNameChanged(int, QString)
{
}

} // namespace model

