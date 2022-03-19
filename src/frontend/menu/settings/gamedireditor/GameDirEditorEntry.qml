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


Rectangle {
    id: root

    readonly property bool highlighted: ListView.view.focus
                                        && (ListView.isCurrentItem || mouseArea.containsMouse)
    property bool selected: false

    signal pressed

    width: parent.width
    height: label.height
    color: highlighted ? "#585858" : "transparent"

    Keys.onPressed: {
        if (api.keys.isAccept(event) && !event.isAutoRepeat) {
            event.accepted = true;
            root.pressed();
        }
    }


    Rectangle {
        anchors.fill: parent
        color: "#d55"
        visible: parent.selected
    }

    Text {
        id: label
        text: modelData
        verticalAlignment: Text.AlignVCenter
        lineHeight: 2

        color: "#eee"
        font.family: globalFonts.sans
        font.pixelSize: vpx(18)

        width: parent.width
        leftPadding: parent.height * 0.5
        rightPadding: leftPadding
        elide: Text.ElideRight
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
    }
}
