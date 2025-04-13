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


FocusScope {
    id: root

    property alias label: label.text

    readonly property int fontSize: vpx(22)
    readonly property int horizontalPadding: vpx(30)

    signal activate()


    width: parent.width
    height: fontSize * 2.5
    opacity: enabled ? 1.0 : 0.25

    Keys.onPressed: event => {
        if (api.keys.isAccept(event) && !event.isAutoRepeat) {
            event.accepted = true;
            root.activate();
        }
    }


    Rectangle {
        id: underline

        width: parent.width
        height: vpx(3)
        anchors.bottom: parent.bottom

        color: "#3aa"
        visible: parent.focus || mouseArea.containsMouse
    }

    Text {
        id: label

        anchors.left: parent.left
        anchors.leftMargin: horizontalPadding
        anchors.verticalCenter: parent.verticalCenter

        color: "#eee"
        font.pixelSize: fontSize
        font.family: globalFonts.sans
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked: activate()
        cursorShape: Qt.PointingHandCursor
    }
}
