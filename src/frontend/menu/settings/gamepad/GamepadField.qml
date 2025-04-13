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


import QtQuick 2.15

Item {
    id: root

    property string icon
    property string label
    property string mapping
    property bool highlighted

    readonly property int textSizePx: vpx(24)
    readonly property int imagePadding: height * 0.1
    readonly property int horizontalPadding: vpx(20)
    readonly property color textColor: "#eee"

    signal selected

    width: ListView.view.width
    height: vpx(48)


    Keys.onPressed: event => {
        if (api.keys.isAccept(event) && !event.isAutoRepeat) {
            event.accepted = true;
            root.selected();
        }
    }


    Row {
        height: parent.height

        anchors.left: parent.left
        anchors.leftMargin: root.horizontalPadding

        Item {
            height: parent.height
            width: height

            Image {
                source: root.icon && "qrc:/frontend/assets/gamepad/x360/" + root.icon + ".png"

                anchors.fill: parent
                anchors.margins: root.imagePadding
                anchors.verticalCenter: parent.verticalCenter
                fillMode: Image.PreserveAspectFit
            }
        }

        Text {
            color: root.textColor
            text: root.label
            font.family: globalFonts.condensed
            font.pixelSize: root.textSizePx

            leftPadding: root.imagePadding
            anchors.verticalCenter: parent.verticalCenter
        }
    }

    Text {
        color: root.textColor
        text: root.mapping
        font.family: globalFonts.condensed
        font.pixelSize: root.textSizePx

        anchors.right: parent.right
        anchors.rightMargin: root.horizontalPadding + root.imagePadding
        anchors.verticalCenter: parent.verticalCenter
    }


    Rectangle {
        anchors.fill: parent
        color: "#404040"
        z: -1
        visible: parent.highlighted || mouseArea.containsMouse
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
    }
}
