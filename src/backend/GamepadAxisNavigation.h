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


#pragma once

#include "utils/HashMap.h"

#include <QGamepadManager>
#include <QObject>


class GamepadAxisNavigation : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(GamepadAxisNavigation)

public:
    explicit GamepadAxisNavigation(QObject* parent = nullptr);

public slots:
    void onAxisEvent(int deviceId, QGamepadManager::GamepadAxis axis, double axisValue);

private:
    enum class AxisMovement : unsigned char {
        IN_DEADZONE,
        MOVE_POS, // the axis value has increased
        MOVE_NEG, // the axis value has decreased
    };

    struct AxisState {
        double value;
        AxisMovement move;

        AxisState();
    };

    using DeviceAxes = HashMap<QGamepadManager::GamepadAxis, AxisState, EnumHash>;
    HashMap<int, DeviceAxes> devices;
};
