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


import QtQuick 2.0


Item {
    property alias text: title.text

    width: parent.width
    height: vpx(80)

    Rectangle {
        anchors.fill: parent
        color: "#222"
        opacity: 0.75
    }

    Text {
        id: title

        color: "#eee"
        font.pixelSize: vpx(40)
        font.family: globalFonts.sans

        anchors.left: parent.horizontalCenter
        anchors.leftMargin: parent.width * -0.4
        anchors.bottom: parent.bottom
        anchors.bottomMargin: vpx(10)
    }

    Rectangle {
        width: parent.width * 0.97
        height: vpx(1)
        color: "#555"

        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
    }
}
