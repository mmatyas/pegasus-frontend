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
    property alias text: title.text

    anchors.left: parent.left
    anchors.right: parent.right
    anchors.top: parent.top
    height: vpx(74)

    z: 2

    Text {
        id: title

        color: "#eee"
        font.pixelSize: vpx(36)
        font.family: global.fonts.condensed
        // font.bold: true

        anchors.left: parent.left
        anchors.leftMargin: parent.width * 0.08
        anchors.verticalCenter: parent.verticalCenter
    }

    Rectangle {
        width: parent.width * 0.97
        height: vpx(1)
        color: "#555"

        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
    }
}
