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
    property alias text: label.text
    property alias color: bgRect.color
    signal pressed

    readonly property bool highlighted: mouseArea.containsMouse || activeFocus

    Keys.onPressed: {
        if (api.keys.isAccept(event)) {
            event.accepted = true;
            pressed();
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: parent.pressed()
    }
    Rectangle {
        id: bgRect
        anchors.fill: parent
        opacity: parent.highlighted ? 0.75 : 0.25
    }
    Text {
        id: label
        color: "#eee"
        font.family: globalFonts.sans
        font.pixelSize: root.textSize
        anchors.centerIn: parent
    }
}
