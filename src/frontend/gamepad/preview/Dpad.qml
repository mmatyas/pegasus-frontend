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
    property Gamepad gamepad

    Image {
        z: 100
        anchors.fill: parent

        fillMode: Image.PreserveAspectFit
        source: "/gamepad/dpad.svg"
        sourceSize {
            width: 128
            height: 128
        }
    }

    DpadHighlight {
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        visible: gamepad.buttonLeft
    }
    DpadHighlight {
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        visible: gamepad.buttonRight
    }
    DpadHighlight {
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        visible: gamepad.buttonUp
    }
    DpadHighlight {
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        visible: gamepad.buttonDown
    }
}
