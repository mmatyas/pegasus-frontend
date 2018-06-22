// Pegasus Frontend
// Copyright (C) 2018  Mátyás Mustoha
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


Rectangle {
    id: root

    signal clicked()

    property alias text: label.text
    property double lineHeight: 1.75

    color: focus ? "#4ae" : (mouseArea.containsMouse ? "#999" : "#aaa")
    width: parent.width
    height: label.height * lineHeight
    radius: vpx(3)
    //border.color: "#0870aa"
    border.width: vpx(1)

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        onClicked: root.clicked()
        hoverEnabled: true
    }

    Text {
        id: label
        color: root.focus ? "#eee" : "#666"
        font {
            pixelSize: vpx(18)
            family: globalFonts.sans
            bold: true
        }
        anchors.centerIn: parent
    }
}
