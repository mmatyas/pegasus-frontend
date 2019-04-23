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


#include "GamepadAxisNavigation.h"

using QGamepadButton = QGamepadManager::GamepadButton;
using QGamepadAxis = QGamepadManager::GamepadAxis;
using DeviceAxes = HashMap<QGamepadAxis, double, EnumHash>;


namespace {
enum class AxisZone : unsigned char {
    DEAD, // center
    POSITIVE,
    NEGATIVE,
};

AxisZone axis_zone(double axis_value)
{
    static constexpr double DEADZONE = 0.5;

    if (-DEADZONE < axis_value && axis_value < DEADZONE)
        return AxisZone::DEAD;

    return axis_value > 0
        ? AxisZone::POSITIVE
        : AxisZone::NEGATIVE;
}

QGamepadButton axis_valchange_to_button(QGamepadAxis axis, double axis_change)
{
    const bool is_negative = (axis_change < 0);
    const bool is_horizontal = (axis == QGamepadManager::AxisLeftX ||
                                axis == QGamepadManager::AxisRightX);
    return is_horizontal
        ? (is_negative ? QGamepadButton::ButtonLeft : QGamepadButton::ButtonRight)
        : (is_negative ? QGamepadButton::ButtonUp : QGamepadButton::ButtonDown);
}

QGamepadButton reverse_button(QGamepadButton button)
{
    switch (button) {
        case QGamepadButton::ButtonLeft: return QGamepadButton::ButtonRight;
        case QGamepadButton::ButtonRight: return QGamepadButton::ButtonLeft;
        case QGamepadButton::ButtonUp: return QGamepadButton::ButtonDown;
        case QGamepadButton::ButtonDown: return QGamepadButton::ButtonUp;
        default: Q_UNREACHABLE();
    }
}

double axis_val_or_zero(const DeviceAxes& axes, QGamepadAxis axis)
{
    const auto it = axes.find(axis);
    if (it != axes.cend())
        return it->second;

    return 0.0;
}
} // namespace


GamepadAxisNavigation::GamepadAxisNavigation(QObject* parent)
    : QObject(parent)
{}

void GamepadAxisNavigation::onAxisEvent(int deviceId, QGamepadAxis axis, double axisValue)
{
    if (axis == QGamepadAxis::AxisInvalid)
        return;

    // NOTE: the point here is that if the device or axis wasn't
    // registered yet, it will be created automatically
    DeviceAxes& device_axes = devices[deviceId];
    const double prev_value = axis_val_or_zero(device_axes, axis);
    device_axes[axis] = axisValue;

    const AxisZone prev_zone = axis_zone(prev_value);
    const AxisZone curr_zone = axis_zone(axisValue);
    if (prev_zone == curr_zone)
        return;

    const double val_change = axisValue - prev_value;
    const QGamepadButton pressed_btn = axis_valchange_to_button(axis, val_change);
    const QGamepadButton released_btn = reverse_button(pressed_btn);

    if (prev_zone != AxisZone::DEAD)
        emit buttonReleased(deviceId, released_btn);

    if (curr_zone != AxisZone::DEAD)
        emit buttonPressed(deviceId, pressed_btn);
}
