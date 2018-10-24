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

Item {
    property string side
    property bool pressed: false
    property real xPercent: 0.0
    property real yPercent: 0.0

    height: width

    Rectangle {
        // basement
        width: parent.width * 0.64
        height: width
        anchors.centerIn: parent

        color: "#222"
        border { width: 1.5; color: "#aaa" }
        radius: width * 0.5
    }

    Image {
        width: parent.width * 0.7
        anchors {
            centerIn: parent
            horizontalCenterOffset: parent.width * 0.15 * xPercent
            verticalCenterOffset: parent.width * 0.15 * yPercent
        }

        fillMode: Image.PreserveAspectFit
        source: "qrc:/frontend/assets/gamepad/stick.svg"
        sourceSize {
            width: 128
            height: 128
        }

        transform: [
            Rotation {
                origin.x: width * 0.5; origin.y: height * 0.5
                axis { x: 0; y: 1; z: 0 }
                angle: xPercent * 35
            },
            Rotation {
                origin.x: width * 0.5; origin.y: height * 0.5
                axis { x: 1; y: 0; z: 0 }
                angle: yPercent * 35
            }
        ]
    }

    Rectangle {
        // L3/R3 highlight
        width: parent.width * 0.4
        height: width
        anchors.centerIn: parent

        color: pressed ? "#393": "#3cc"
        radius: width * 0.5

        visible: pressed || padContainer.currentButton === (side + "3")
    }
    Rectangle {
        id: highlightX
        width: parent.width * 0.9
        height: vpx(2)
        anchors.centerIn: parent

        color: "#3cc"
        visible: padContainer.currentButton === (side + "x")
    }
    Rectangle {
        // highlightY
        width: highlightX.height
        height: highlightX.width
        anchors.centerIn: parent

        color: highlightX.color
        visible: padContainer.currentButton === (side + "y")
    }
}
