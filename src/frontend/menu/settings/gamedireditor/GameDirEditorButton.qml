// Pegasus Frontend
// Copyright (C) 2017-2022  Mátyás Mustoha
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


Item {
    id: root

    property color color
    property string icon

    readonly property bool highlighted: focus || mouseArea.containsMouse

    signal pressed

    width: vpx(80)
    height: width * 1.5

    Keys.onPressed: event => {
        if (api.keys.isAccept(event) && !event.isAutoRepeat) {
            event.accepted = true;
            root.pressed();
        }
    }


    Rectangle {
        color: root.color
        radius: vpx(5)
        anchors.fill: parent
        opacity: root.highlighted ? 1.0 : 0.7
    }

    Text {
        text: root.icon
        font.family: globalFonts.sans
        font.pixelSize: root.width * 0.45
        font.bold: true
        color: "#eee"
        anchors.centerIn: parent
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onPressed: root.pressed()
    }
}
