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


import QtQuick 2.7

Row {
    id: root

    property alias fontSize: label.font.pixelSize
    property alias text: label.text
    property alias textColor: label.color

    property bool checked: false

    spacing: vpx(8)

    Rectangle {
        width: parent.fontSize * 1.4
        height: width

        color: "#50000000"
        border.color: "#60000000"
        border.width: vpx(1)

        anchors.verticalCenter: parent.verticalCenter

        Text {
            text: root.checked ? "\xd7" : ""
            color: textColor
            font {
                bold: true
                pixelSize: parent.height
                family: globalFonts.sans
            }

            anchors.fill: parent
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }

        MouseArea {
            anchors.fill: parent
            onClicked: root.checked = !root.checked
        }
    }

    Text {
        id: label

        height: parent.height
        verticalAlignment: Text.AlignVCenter

        color: "#eee"
        font {
            bold: root.activeFocus
            family: globalFonts.sans
        }
    }

    Keys.onPressed: {
        if (api.keys.isAccept(event.key) && !event.isAutoRepeat) {
            event.accepted = true;
            checked = !checked;
        }
    }
}
