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
import QtGraphicalEffects 1.0

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
        KeyNavigation.down: layoutArea
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

    Rectangle {
        id: layoutArea
        color: "#222"
        width: parent.width
        anchors {
            top: deviceSelect.bottom
            bottom: parent.bottom
        }

        KeyNavigation.up: deviceSelect

        Item {
            id: padContainer
            anchors.fill: parent

            Image {
                id: padBase
                width: parent.width
                height: rpx(320)
                anchors.centerIn: parent

                fillMode: Image.PreserveAspectFit
                source: "/gamepad/base.svg"
                sourceSize {
                    width: 512
                    height: 512
                }
            }
            Item {
                width: padSelect.width + padGuide.width + padStart.width + 10
                height: padGuide.height
                anchors {
                    verticalCenter: padBase.verticalCenter
                    verticalCenterOffset: -rpx(25)
                    horizontalCenter: padBase.horizontalCenter
                }
                GamepadPiece {
                    id: padSelect
                    width: rpx(38)
                    anchors {
                        left: parent.left
                        verticalCenter: parent.verticalCenter
                    }

                    source: "/gamepad/select.svg"
                    active: gamepad.buttonSelect
                }
                GamepadPiece {
                    id: padStart
                    width: rpx(38)
                    anchors {
                        right: parent.right
                        verticalCenter: parent.verticalCenter
                    }

                    source: "/gamepad/start.svg"
                    active: gamepad.buttonStart
                }
                GamepadPiece {
                    id: padGuide
                    width: rpx(50)
                    anchors.centerIn: parent
                    source: "/gamepad/guide.svg"
                    active: gamepad.buttonStart
                }
            }
            Item {
                id: padABXYArea
                width: padA.width * 3
                height: width
                anchors {
                    verticalCenter: padBase.verticalCenter
                    verticalCenterOffset: rpx(-12)
                    left: padBase.horizontalCenter
                    leftMargin: rpx(107)
                }
                GamepadPiece {
                    id: padA
                    width: rpx(40)
                    anchors.bottom: parent.bottom
                    anchors.horizontalCenter: parent.horizontalCenter
                    source: "/gamepad/a.svg"
                    active: gamepad.buttonA
                }
                GamepadPiece {
                    id: padB
                    width: padA.width
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    source: "/gamepad/b.svg"
                    active: gamepad.buttonB
                }
                GamepadPiece {
                    id: padX
                    width: padA.width
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    source: "/gamepad/x.svg"
                    active: gamepad.buttonX
                }
                GamepadPiece {
                    id: padY
                    width: padA.width
                    anchors.top: parent.top
                    anchors.horizontalCenter: parent.horizontalCenter
                    source: "/gamepad/y.svg"
                    active: gamepad.buttonY
                }
            }
            Item {
                id: padDpadArea
                width: padABXYArea.width * 0.95
                height: width
                anchors {
                    verticalCenter: padBase.verticalCenter
                    verticalCenterOffset: padABXYArea.anchors.verticalCenterOffset
                    right: padBase.horizontalCenter
                    rightMargin: padABXYArea.anchors.leftMargin
                }

                GamepadDpad {
                    id: padDpad
                    anchors.fill: parent
                    gamepad: gamepad
                }
            }
        }

        ColorOverlay {
            anchors.fill: padContainer
            source: padContainer
            color: "#ffaaaaaa"
        }
    }

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.RightButton
        onClicked: screenClosed()
    }
}
