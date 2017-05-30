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
        Keys.forwardTo: [gamepadList]
        KeyNavigation.down: configL1

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

        property int horizontalOffset: rpx(-560)
        property int verticalSpacing: rpx(170)

        Rectangle {
            color: "#222"
            anchors.fill: parent
        }

        GamepadConfigGroup {
            groupName: qsTr("left back")
            anchors {
                left: parent.horizontalCenter
                leftMargin: parent.horizontalOffset
                verticalCenter: parent.verticalCenter
                verticalCenterOffset: -parent.verticalSpacing
            }

            GamepadConfigField {
                focus: true
                id: configL1
                text: qsTr("shoulder")

                KeyNavigation.right: configR1
                KeyNavigation.down: configL2
            }
            GamepadConfigField {
                id: configL2
                text: qsTr("trigger")

                KeyNavigation.right: configR2
                KeyNavigation.down: configDpadUp
            }
        }

        GamepadConfigGroup {
            groupName: qsTr("dpad")
            anchors {
                left: parent.horizontalCenter
                leftMargin: parent.horizontalOffset
                verticalCenter: parent.verticalCenter
            }

            GamepadConfigField {
                id: configDpadUp
                text: qsTr("up")

                KeyNavigation.right: configA
                KeyNavigation.down: configDpadLeft
            }
            GamepadConfigField {
                id: configDpadLeft
                text: qsTr("left")

                KeyNavigation.right: configB
                KeyNavigation.down: configDpadRight
            }
            GamepadConfigField {
                id: configDpadRight
                text: qsTr("right")

                KeyNavigation.right: configX
                KeyNavigation.down: configDpadDown
            }
            GamepadConfigField {
                id: configDpadDown
                text: qsTr("down")

                KeyNavigation.right: configY
                KeyNavigation.down: configLeftStickX
            }
        }

        GamepadConfigGroup {
            groupName: qsTr("left stick")
            anchors {
                left: parent.horizontalCenter
                leftMargin: parent.horizontalOffset
                verticalCenter: parent.verticalCenter
                verticalCenterOffset: parent.verticalSpacing
            }

            GamepadConfigField {
                id: configLeftStickX
                text: qsTr("x axis")

                KeyNavigation.right: configRightStickX
                KeyNavigation.down: configLeftStickY
            }
            GamepadConfigField {
                id: configLeftStickY
                text: qsTr("y axis")

                KeyNavigation.right: configRightStickY
                KeyNavigation.down: configL3
            }
            GamepadConfigField {
                id: configL3
                text: qsTr("press")

                KeyNavigation.right: configR3
            }
        }

        GamepadConfigGroup {
            groupName: qsTr("right back")
            alignRight: true
            anchors {
                right: parent.horizontalCenter
                rightMargin: parent.horizontalOffset
                verticalCenter: parent.verticalCenter
                verticalCenterOffset: -parent.verticalSpacing
            }

            GamepadConfigField {
                id: configR1
                text: qsTr("shoulder")

                KeyNavigation.up: deviceSelect
                KeyNavigation.down: configR2
            }
            GamepadConfigField {
                id: configR2
                text: qsTr("trigger")

                KeyNavigation.down: configA
            }
        }

        GamepadConfigGroup {
            groupName: qsTr("abxy")
            alignRight: true
            anchors {
                right: parent.horizontalCenter
                rightMargin: parent.horizontalOffset
                verticalCenter: parent.verticalCenter
            }

            GamepadConfigField {
                id: configA
                text: "a"

                KeyNavigation.down: configB
            }
            GamepadConfigField {
                id: configB
                text: "b"

                KeyNavigation.down: configX
            }
            GamepadConfigField {
                id: configX
                text: "x"

                KeyNavigation.down: configY
            }
            GamepadConfigField {
                id: configY
                text: "y"

                KeyNavigation.down: configRightStickX
            }
        }

        GamepadConfigGroup {
            groupName: qsTr("right stick")
            alignRight: true
            anchors {
                right: parent.horizontalCenter
                rightMargin: parent.horizontalOffset
                verticalCenter: parent.verticalCenter
                verticalCenterOffset: parent.verticalSpacing
            }

            GamepadConfigField {
                id: configRightStickX
                text: qsTr("x axis")

                KeyNavigation.down: configRightStickY
            }
            GamepadConfigField {
                id: configRightStickY
                text: qsTr("y axis")

                KeyNavigation.down: configR3
            }
            GamepadConfigField {
                id: configR3
                text: qsTr("press")
            }
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
