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

Rectangle {
    property color activeColor
    property color inactiveColor
    property bool selected: false

    property int textSizePx
    property alias text: itemLabel.text

    signal activated()

    width: parent.width
    height: textSizePx * 2
    color: selected || mouseArea.containsMouse ? activeColor : inactiveColor

    Keys.onReturnPressed: activated()

    Text {
        id: itemLabel
        anchors {
            right: parent.right; rightMargin: vpx(32)
            verticalCenter: parent.verticalCenter
        }
        color: "#eee"
        font {
            bold: true
            pixelSize: textSizePx
            family: uiFontCondensed.name
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked: parent.activated()
    }
}
