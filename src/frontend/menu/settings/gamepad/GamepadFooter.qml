// Pegasus Frontend
// Copyright (C) 2017-2021  Mátyás Mustoha
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


import QtQuick 2.0

Item {
    id: footer

    anchors.left: parent.left
    anchors.right: parent.right
    anchors.bottom: parent.bottom
    height: vpx(50)

    Rectangle {
        width: parent.width * 0.97
        height: vpx(1)
        color: "#777"
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
    }

    Row {
        anchors.right: parent.right
        anchors.rightMargin: parent.width * 0.08
        anchors.verticalCenter: parent.verticalCenter

        Item {
            height: label.height
            width: height

            Image {
                source: "qrc:/frontend/assets/gamepad/x360/b.png"

                anchors.fill: parent
                anchors.margins: parent.height * 0.1
                anchors.verticalCenter: parent.verticalCenter
                fillMode: Image.PreserveAspectFit
            }
        }

        Text {
            id: label

            text: qsTr("Back") + api.tr
            color: "#777"

            font.family: global.fonts.condensed
            font.pixelSize: vpx(22)
            font.capitalization: Font.AllUppercase

            lineHeight: 1.5
            verticalAlignment: Text.AlignVCenter
        }
    }
}
