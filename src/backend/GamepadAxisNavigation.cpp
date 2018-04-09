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


#include "GamepadAxisNavigation.h"

#include <QGuiApplication>
#include <QKeyEvent>
#include <QWindow>

#include <QDebug>

using GamepadAxis = QGamepadManager::GamepadAxis;


namespace {
void emitKey(Qt::Key key, QEvent::Type eventType)
{
    if (key == Qt::Key_unknown || eventType == QEvent::None)
        return;

    Q_ASSERT(eventType == QEvent::KeyPress || eventType == QEvent::KeyRelease);


    QKeyEvent event(eventType, key, Qt::NoModifier);

    QWindow* focusWindow = qApp ? qApp->focusWindow() : nullptr;
    if (focusWindow)
        QGuiApplication::sendEvent(focusWindow, &event);
}
} // namespace


GamepadAxisNavigation::AxisState::AxisState()
    : value(0.0)
    , move(AxisMovement::IN_DEADZONE)
{
}

GamepadAxisNavigation::GamepadAxisNavigation(QObject* parent)
    : QObject(parent)
{
}

void GamepadAxisNavigation::onAxisEvent(int deviceId, GamepadAxis axis, double axisValue)
{
    if (axis == GamepadAxis::AxisInvalid)
        return;

    // NOTE: the point here is that if the device or axis wasn't
    // registered yet, it will be created automatically
    DeviceAxes& device_axes = devices[deviceId];
    AxisState& prev_state = device_axes[axis];

    // default values
    AxisMovement current_move = AxisMovement::IN_DEADZONE;
    Qt::Key event_key = Qt::Key_unknown;
    QEvent::Type event_type = QEvent::None;


    static constexpr double DEADZONE = 0.5;
    const bool inside_deadzone = (-DEADZONE < axisValue && axisValue < DEADZONE);

    if (!inside_deadzone) {
        // the current state depends on the relative movement
        current_move = (prev_state.value < axisValue)
            ? AxisMovement::MOVE_POS
            : AxisMovement::MOVE_NEG;

        // the emitted event type depends on the change in the state
        if (current_move != prev_state.move) {
            event_type = (prev_state.move == AxisMovement::IN_DEADZONE)
                ? QEvent::KeyPress
                : QEvent::KeyRelease;
        }

        // the emitted key depends on the absolute values
        const bool is_negative = (axisValue < 0);
        const bool is_horizontal = (axis == QGamepadManager::AxisLeftX ||
                                    axis == QGamepadManager::AxisRightX);
        event_key = is_horizontal
                    ? (is_negative ? Qt::Key_Left : Qt::Key_Right)
                    : (is_negative ? Qt::Key_Up : Qt::Key_Down);
    }


    // store the current values for the next run
    if (!inside_deadzone) {
        prev_state.value = axisValue;
        prev_state.move = current_move;
    }
    else {
        prev_state.value = 0.0;
        prev_state.move = AxisMovement::IN_DEADZONE;
    }

    // emit the event
    emitKey(event_key, event_type);
}
