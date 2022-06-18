// Pegasus Frontend
// Copyright (C) 2017-2022  Mátyás Mustoha
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


#include "GamepadListModel.h"

#include "Gamepad.h"


namespace {
enum Roles {
    Self = Qt::UserRole,
    Name,
    DeviceId,

    ButtonUp,
    ButtonDown,
    ButtonLeft,
    ButtonRight,

    ButtonNorth,
    ButtonSouth,
    ButtonEast,
    ButtonWest,

    ButtonA,
    ButtonB,
    ButtonC,
    ButtonX,
    ButtonY,
    ButtonZ,

    ButtonL1,
    ButtonL2,
    ButtonL3,
    ButtonR1,
    ButtonR2,
    ButtonR3,

    ButtonSelect,
    ButtonStart,
    ButtonGuide,

    AxisLeftX,
    AxisRightX,
    AxisLeftY,
    AxisRightY,
};
} // namespace



namespace model {
GamepadListModel::GamepadListModel(QObject* parent)
    : TypeListModel(parent)
{}


QHash<int, QByteArray> GamepadListModel::roleNames() const
{
    static QHash<int, QByteArray> ROLE_NAMES {
        { Roles::Self, QByteArrayLiteral("modelData") },
        { Roles::Name, QByteArrayLiteral("name") },
        { Roles::DeviceId, QByteArrayLiteral("deviceId") },
        { Roles::ButtonUp, QByteArrayLiteral("buttonUp") },
        { Roles::ButtonDown, QByteArrayLiteral("buttonDown") },
        { Roles::ButtonLeft, QByteArrayLiteral("buttonLeft") },
        { Roles::ButtonRight, QByteArrayLiteral("buttonRight") },
        { Roles::ButtonNorth, QByteArrayLiteral("buttonNorth") },
        { Roles::ButtonSouth, QByteArrayLiteral("buttonSouth") },
        { Roles::ButtonEast, QByteArrayLiteral("buttonEast") },
        { Roles::ButtonWest, QByteArrayLiteral("buttonWest") },
        { Roles::ButtonL1, QByteArrayLiteral("buttonL1") },
        { Roles::ButtonL2, QByteArrayLiteral("buttonL2") },
        { Roles::ButtonL3, QByteArrayLiteral("buttonL3") },
        { Roles::ButtonR1, QByteArrayLiteral("buttonR1") },
        { Roles::ButtonR2, QByteArrayLiteral("buttonR2") },
        { Roles::ButtonR3, QByteArrayLiteral("buttonR3") },
        { Roles::ButtonSelect, QByteArrayLiteral("buttonSelect") },
        { Roles::ButtonStart, QByteArrayLiteral("buttonStart") },
        { Roles::ButtonGuide, QByteArrayLiteral("buttonGuide") },
        { Roles::AxisLeftX, QByteArrayLiteral("axisLeftX") },
        { Roles::AxisRightX, QByteArrayLiteral("axisRightX") },
        { Roles::AxisLeftY, QByteArrayLiteral("axisLeftY") },
        { Roles::AxisRightY, QByteArrayLiteral("axisRightY") },
    };
    return ROLE_NAMES;
}


QVariant GamepadListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return {};

    model::Gamepad* const gamepad_ptr = m_entries.at(index.row());
    const model::Gamepad& gamepad = *gamepad_ptr;
    switch (role) {
        case Roles::Self: return QVariant::fromValue(gamepad_ptr);
        case Roles::Name: return gamepad.name();
        case Roles::DeviceId: return gamepad.deviceId();
        case Roles::ButtonUp: return gamepad.buttonUp();
        case Roles::ButtonDown: return gamepad.buttonDown();
        case Roles::ButtonLeft: return gamepad.buttonLeft();
        case Roles::ButtonRight: return gamepad.buttonRight();
        case Roles::ButtonA: return gamepad.buttonA();
        case Roles::ButtonB: return gamepad.buttonB();
        case Roles::ButtonC: return gamepad.buttonC();
        case Roles::ButtonX: return gamepad.buttonX();
        case Roles::ButtonY: return gamepad.buttonY();
        case Roles::ButtonZ: return gamepad.buttonZ();
        case Roles::ButtonEast: return gamepad.buttonX();
        case Roles::ButtonNorth: return gamepad.buttonY();
        case Roles::ButtonSouth: return gamepad.buttonA();
        case Roles::ButtonWest: return gamepad.buttonB();
        case Roles::ButtonL1: return gamepad.buttonL1();
        case Roles::ButtonL2: return gamepad.buttonL2();
        case Roles::ButtonL3: return gamepad.buttonL3();
        case Roles::ButtonR1: return gamepad.buttonR1();
        case Roles::ButtonR2: return gamepad.buttonR2();
        case Roles::ButtonR3: return gamepad.buttonR3();
        case Roles::ButtonSelect: return gamepad.buttonSelect();
        case Roles::ButtonStart: return gamepad.buttonStart();
        case Roles::ButtonGuide: return gamepad.buttonGuide();
        case Roles::AxisLeftX: return gamepad.axisLeftX();
        case Roles::AxisRightX: return gamepad.axisRightX();
        case Roles::AxisLeftY: return gamepad.axisLeftY();
        case Roles::AxisRightY: return gamepad.axisRightY();
        default: return {};
    }
}


model::Gamepad* GamepadListModel::findById(int device_id) const
{
    const auto pred = [device_id](const model::Gamepad* const gp){ return gp->deviceId() == device_id; };
    const auto it = std::find_if(m_entries.begin(), m_entries.end(), pred);
    return it == m_entries.end()
        ? nullptr
        : *it;
}


void GamepadListModel::append(model::Gamepad* item)
{
    beginInsertRows(QModelIndex(), count(), count());
    m_entries.emplace_back(item);
    endInsertRows();

    emit countChanged();
}


void GamepadListModel::remove(model::Gamepad* item)
{
    const int data_idx = indexOf(item);
    if (data_idx < 0)
        return;

    beginRemoveRows(QModelIndex(), data_idx, data_idx);
    m_entries.erase(m_entries.begin() + data_idx);
    endRemoveRows();

    emit countChanged();
}
} // namespace model
