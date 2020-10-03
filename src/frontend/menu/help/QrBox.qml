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


import QtQuick 2.6


Rectangle {
    id: root

    property alias url: mUrl.text
    property alias image: mImage.source

    color: mouse.containsMouse ? border.color : "transparent"
    border.color: "#007878"
    border.width: vpx(2)
    radius: vpx(8)

    width: row.width
    height: row.height


    MouseArea {
        id: mouse
        anchors.fill: parent

        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor

        onClicked: Qt.openUrlExternally("http://" + root.url)
    }


    Row {
        id: row
        height: mUrl.height

        Rectangle {
            color: "white"
            width: parent.height; height: width
            radius: root.radius

            Image {
                id: mImage
                smooth: false

                width: parent.width - parent.radius * 1.5; height: width
                anchors.centerIn: parent
            }
        }

        Text {
            id: mUrl

            color: "#eee"
            font.pixelSize: vpx(18)
            font.family: global.fonts.mono

            height: font.pixelSize * 5
            verticalAlignment: Text.AlignVCenter
            leftPadding: font.pixelSize * 1.5
            rightPadding: leftPadding
        }
    }
}
