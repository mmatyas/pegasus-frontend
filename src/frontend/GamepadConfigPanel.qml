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


import QtQuick 2.8
import QtGamepad 1.0

FocusScope {
    property bool hasGamepads: GamepadManager.connectedGamepads.length > 0

    signal screenClosed()

    width: parent.width
    height: parent.height
    visible: x < parent.width

    Keys.onEscapePressed: screenClosed()


    Gamepad {
        id: gamepad
        deviceId: -1
    }

    Rectangle {
        id: deviceSelect
        width: parent.width
        height: rpx(70)
        color: "#333"
        anchors.top: parent.top

        focus: true
        Keys.onDownPressed: layoutArea.forceActiveFocus()
        Keys.forwardTo: [gamepadList]

        GamepadName {
            visible: !hasGamepads
            highlighted: deviceSelect.focus
            text: qsTr("No gamepads connected")
        }

        ListView {
            id: gamepadList
            anchors.fill: parent

            clip: true
            highlightRangeMode: ListView.StrictlyEnforceRange
            highlightMoveDuration: 300
            orientation: ListView.Horizontal

            // FIXME: it seems Qt 5.8 can't list the connected gamepads
            // model: GamepadManager.connectedGamepads
            model: GamepadManager.connectedGamepads.length

            onCurrentIndexChanged: {
                gamepad.deviceId = GamepadManager.connectedGamepads.length > currentIndex
                                 ? GamepadManager.connectedGamepads[currentIndex] : -1
            }

            delegate: Item {
                width: ListView.view.width
                height: ListView.view.height

                GamepadName {
                    // FIXME: it seems Qt 5.8 doesn't know the name of the gamepad
                    text: "Gamepad name here #" + (index + 1)
                    highlighted: deviceSelect.focus
                }
            }
        }
    }

    FocusScope {
        id: layoutArea
        width: parent.width
        anchors {
            top: deviceSelect.bottom
            bottom: parent.bottom
        }

        Rectangle {
            color: "#222"
            anchors.fill: parent
        }

        GamepadConfigLeftBack {
            id: configLeftBack
            focus: true
            gamepad: gamepad
            anchors {
                right: parent.horizontalCenter
                rightMargin: configDpad.anchors.rightMargin
                verticalCenter: parent.verticalCenter
                verticalCenterOffset: rpx(-170)
            }

            onExitUp: deviceSelect.forceActiveFocus()
            onExitDown: configDpad.forceActiveFocus()
            KeyNavigation.right: configRightBack
        }

        GamepadConfigDpad {
            id: configDpad
            gamepad: gamepad
            anchors {
                right: parent.horizontalCenter; rightMargin: rpx(410)
                verticalCenter: parent.verticalCenter
            }

            onExitUp: configLeftBack.forceActiveFocus()
            onExitDown: configLeftStick.forceActiveFocus()
            KeyNavigation.right: configABXY
        }

        GamepadConfigLeftStick {
            id: configLeftStick
            gamepad: gamepad
            anchors {
                right: parent.horizontalCenter
                rightMargin: configDpad.anchors.rightMargin
                verticalCenter: parent.verticalCenter
                verticalCenterOffset: rpx(170)
            }

            onExitUp: configDpad.forceActiveFocus()
            onExitDown: configLeftBack.forceActiveFocus()
            KeyNavigation.right: configRightStick
        }

        GamepadConfigRightBack {
            id: configRightBack
            focus: true
            gamepad: gamepad
            anchors {
                left: parent.horizontalCenter
                leftMargin: configLeftBack.anchors.rightMargin
                verticalCenter: parent.verticalCenter
                verticalCenterOffset: configLeftBack.anchors.verticalCenterOffset
            }

            onExitUp: deviceSelect.forceActiveFocus()
            onExitDown: configABXY.forceActiveFocus()
        }

        GamepadConfigABXY {
            id: configABXY
            gamepad: gamepad
            anchors {
                left: parent.horizontalCenter
                leftMargin: configDpad.anchors.rightMargin
                verticalCenter: parent.verticalCenter
            }

            onExitUp: configRightBack.forceActiveFocus()
            onExitDown: configRightStick.forceActiveFocus()
        }

        GamepadConfigRightStick {
            id: configRightStick
            gamepad: gamepad
            anchors {
                left: parent.horizontalCenter
                leftMargin: configLeftStick.anchors.rightMargin
                verticalCenter: parent.verticalCenter
                verticalCenterOffset: configLeftStick.anchors.verticalCenterOffset
            }

            onExitUp: configABXY.forceActiveFocus()
            onExitDown: configRightBack.forceActiveFocus()
        }

        GamepadLayoutPreview {
            gamepad: gamepad
        }
    }

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.RightButton
        onClicked: screenClosed()
    }
}
