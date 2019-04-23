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


#pragma once

#include "utils/HashMap.h"

#include <QGamepadManager>
#include <QObject>


class GamepadAxisNavigation : public QObject {
    Q_OBJECT

public:
    explicit GamepadAxisNavigation(QObject* parent = nullptr);

public slots:
    void onAxisEvent(int deviceId, QGamepadManager::GamepadAxis axis, double axisValue);

signals:
    void buttonPressed(int deviceId, QGamepadManager::GamepadButton button);
    void buttonReleased(int deviceId, QGamepadManager::GamepadButton button);

private:
    using DeviceAxes = HashMap<QGamepadManager::GamepadAxis, double, EnumHash>;
    HashMap<int, DeviceAxes> devices;
};
