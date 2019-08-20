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


import QtQuick 2.8

Item {
    id: padContainer
    width: parent.width
    height: padBase.height
    anchors.verticalCenter: parent.verticalCenter
    anchors.verticalCenterOffset: vpx(45)

    property var gamepad
    property string currentButton: ""


    Image {
        id: padBase
        width: parent.width
        height: vpx(320)
        anchors.centerIn: parent

        fillMode: Image.PreserveAspectFit
        source: "qrc:/frontend/assets/gamepad/base.svg"
        sourceSize {
            width: 512
            height: 512
        }
    }

    PadTrigger {
        id: padL2
        width: vpx(50)
        anchors {
            bottom: padBase.verticalCenter
            bottomMargin: vpx(113)
            right: padBase.horizontalCenter
            rightMargin: vpx(131)
        }

        shortName: "l2"
        pressed: gamepad && gamepad.buttonL2
    }
    PadShoulder {
        id: padL1
        width: vpx(110)
        anchors {
            bottom: padBase.verticalCenter
            bottomMargin: vpx(84)
            right: padBase.horizontalCenter
            rightMargin: vpx(110)
        }

        shortName: "l1"
        pressed: gamepad && gamepad.buttonL1
    }
    PadTrigger {
        id: padR2
        width: padL2.width
        anchors {
            bottom: padBase.verticalCenter
            bottomMargin: padL2.anchors.bottomMargin
            left: padBase.horizontalCenter
            leftMargin: padL2.anchors.rightMargin
        }

        shortName: "r2"
        pressed: gamepad && gamepad.buttonR2
    }
    PadShoulder {
        id: padR1
        width: padL1.width
        anchors {
            bottom: padBase.verticalCenter
            bottomMargin: padL1.anchors.bottomMargin
            left: padBase.horizontalCenter
            leftMargin: padL1.anchors.rightMargin
        }

        shortName: "r1"
        pressed: gamepad && gamepad.buttonR1
    }

    Item {
        width: padSelect.width + padGuide.width + padStart.width + 10
        height: padGuide.height
        anchors {
            verticalCenter: padBase.verticalCenter
            verticalCenterOffset: vpx(-20)
            horizontalCenter: padBase.horizontalCenter
        }
        PadButton {
            id: padSelect
            width: vpx(38)
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter

            shortName: "select"
            pressed: gamepad && gamepad.buttonSelect
        }
        PadButton {
            id: padStart
            width: vpx(38)
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter

            shortName: "start"
            pressed: gamepad && gamepad.buttonStart
        }
        PadButton {
            id: padGuide
            width: vpx(50)
            anchors.centerIn: parent

            shortName: "guide"
            pressed: gamepad && gamepad.buttonGuide
        }
    }

    Item {
        id: padABXYArea
        width: padA.width * 3
        height: width
        anchors {
            verticalCenter: padBase.verticalCenter
            verticalCenterOffset: vpx(-15)
            left: padBase.horizontalCenter
            leftMargin: vpx(112)
        }
        PadButton {
            id: padA
            width: vpx(38)
            anchors.bottom: parent.bottom
            anchors.horizontalCenter: parent.horizontalCenter

            shortName: "a"
            pressed: gamepad && gamepad.buttonA
        }
        PadButton {
            id: padB
            width: padA.width
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter

            shortName: "b"
            pressed: gamepad && gamepad.buttonB
        }
        PadButton {
            id: padX
            width: padA.width
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter

            shortName: "x"
            pressed: gamepad && gamepad.buttonX
        }
        PadButton {
            id: padY
            width: padA.width
            anchors.top: parent.top
            anchors.horizontalCenter: parent.horizontalCenter
            shortName: "y"
            pressed: gamepad && gamepad.buttonY
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
        width: vpx(110)
        anchors {
            top: padBase.verticalCenter
            topMargin: vpx(22)
            right: padBase.horizontalCenter
            rightMargin: vpx(18)
        }

        side: "l"
        pressed: gamepad && gamepad.buttonL3
        xPercent: (gamepad && gamepad.axisLeftX) || 0.0
        yPercent: (gamepad && gamepad.axisLeftY) || 0.0
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
        side: "r"
        pressed: gamepad && gamepad.buttonR3
        xPercent: (gamepad && gamepad.axisRightX) || 0.0
        yPercent: (gamepad && gamepad.axisRightY) || 0.0
    }
}
