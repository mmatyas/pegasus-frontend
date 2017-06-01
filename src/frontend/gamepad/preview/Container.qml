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

Item {
    id: padContainer
    anchors.fill: parent

    property Gamepad gamepad


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
            verticalCenterOffset: rpx(-20)
            horizontalCenter: padBase.horizontalCenter
        }
        PadButton {
            id: padSelect
            width: rpx(38)
            anchors {
                left: parent.left
                verticalCenter: parent.verticalCenter
            }

            source: "/gamepad/select.svg"
            active: gamepad.buttonSelect
        }
        PadButton {
            id: padStart
            width: rpx(38)
            anchors {
                right: parent.right
                verticalCenter: parent.verticalCenter
            }

            source: "/gamepad/start.svg"
            active: gamepad.buttonStart
        }
        PadButton {
            id: padGuide
            width: rpx(50)
            anchors.centerIn: parent
            source: "/gamepad/guide.svg"
            active: gamepad.buttonGuide
        }
    }
    Item {
        id: padABXYArea
        width: padA.width * 3
        height: width
        anchors {
            verticalCenter: padBase.verticalCenter
            verticalCenterOffset: rpx(-15)
            left: padBase.horizontalCenter
            leftMargin: rpx(112)
        }
        PadButton {
            id: padA
            width: rpx(38)
            anchors.bottom: parent.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            source: "/gamepad/a.svg"
            active: gamepad.buttonA
        }
        PadButton {
            id: padB
            width: padA.width
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            source: "/gamepad/b.svg"
            active: gamepad.buttonB
        }
        PadButton {
            id: padX
            width: padA.width
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            source: "/gamepad/x.svg"
            active: gamepad.buttonX
        }
        PadButton {
            id: padY
            width: padA.width
            anchors.top: parent.top
            anchors.horizontalCenter: parent.horizontalCenter
            source: "/gamepad/y.svg"
            active: gamepad.buttonY
        }
    }
    Dpad {
        id: padDpadArea
        width: padABXYArea.width * 0.95
        height: width
        anchors {
            verticalCenter: padBase.verticalCenter
            verticalCenterOffset: padABXYArea.anchors.verticalCenterOffset
            right: padBase.horizontalCenter
            rightMargin: padABXYArea.anchors.leftMargin
        }
        gamepad: parent.gamepad
    }
    Stick {
        id: padLeftStick
        width: rpx(110)
        anchors {
            top: padBase.verticalCenter
            topMargin: rpx(22)
            right: padBase.horizontalCenter
            rightMargin: rpx(18)
        }
        active: gamepad.buttonL3
        xPercent: gamepad.axisLeftX
        yPercent: gamepad.axisLeftY
    }
    Stick {
        id: padRightStick
        width: padLeftStick.width
        anchors {
            top: padBase.verticalCenter
            topMargin: padLeftStick.anchors.topMargin
            left: padBase.horizontalCenter
            leftMargin: padLeftStick.anchors.rightMargin
        }
        active: gamepad.buttonR3
        xPercent: gamepad.axisRightX
        yPercent: gamepad.axisRightY
    }
}
