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


FocusScope {
    id: root

    property alias label: label.text
    property alias desc: description.text
    property alias checked: toggle.checked

    property int fontSize: vpx(22)
    property int horizontalPadding: vpx(30)

    width: parent.width
    height: labelContainer.height + fontSize * 1.25
    opacity: enabled ? 1.0 : 0.25

    Rectangle {
        id: underline

        width: parent.width
        height: vpx(3)
        anchors.bottom: parent.bottom

        color: "#3aa"
        visible: parent.focus || mouseArea.containsMouse
    }

    Column {
        id: labelContainer
        anchors {
            left: parent.left; leftMargin: horizontalPadding
            verticalCenter: parent.verticalCenter
        }
        width: parent.width * 0.75 - horizontalPadding

        spacing: fontSize * 0.25
        height: label.height + (description.text ? spacing + description.height : 0)


        Text {
            id: label

            color: "#eee"
            font.pixelSize: fontSize
            font.family: globalFonts.sans
        }

        Text {
            id: description

            color: "#999"
            font.pixelSize: fontSize * 0.8
            font.family: globalFonts.sans

            anchors.left: parent.left
            anchors.right: parent.right
            wrapMode: Text.WordWrap
        }
    }

    Switch {
        id: toggle

        focus: true

        anchors.right: parent.right
        anchors.rightMargin: horizontalPadding
        anchors.verticalCenter: parent.verticalCenter

        height: fontSize * 1.15
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked: toggle.checked = !toggle.checked
        cursorShape: Qt.PointingHandCursor
    }
}
