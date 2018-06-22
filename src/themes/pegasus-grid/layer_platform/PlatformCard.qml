// Pegasus Frontend
// Copyright (C) 2017-2018  Mátyás Mustoha
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
    property alias platformShortName: label.text
    property bool isOnTop: false

    clip: true

    Rectangle {
        id: main
        width: parent.width * 0.8
        height: parent.height
        color: "#ff4035"
    }

    Rectangle {
        width: main.height * 1.7
        height: width
        color: main.color
        rotation: -70
        anchors {
            verticalCenter: parent.top
            horizontalCenter: main.right
            horizontalCenterOffset: vpx(-27)
        }

        Text {
            id: label
            width: 0
            text: ""
            color: "#eee"
            anchors {
                left: parent.left
                bottom: parent.bottom
                leftMargin: vpx(8)
                bottomMargin: vpx(4)
            }
            font {
                bold: true
                capitalization: Font.AllUppercase
                pixelSize: vpx(16)
                family: globalFonts.condensed
            }
        }

        Rectangle {
            color: "#faa"
            width: parent.width
            height: vpx(2)
            anchors.top: parent.bottom
            anchors.left: parent.left
            antialiasing: true
        }
    }

    Image {
        source: "../assets/logos/" + platformShortName + ".svg"
        sourceSize.height: 100
        fillMode: Image.PreserveAspectFit

        asynchronous: true
        opacity: parent.isOnTop ? 1.0 : 0.3

        width: parent.width * 0.6
        height: parent.height - 12
        anchors {
            bottom: parent.bottom
            bottomMargin: vpx(6)
            horizontalCenter: parent.horizontalCenter
            horizontalCenterOffset: vpx(-32)
        }
    }
}
