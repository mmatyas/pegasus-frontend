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
    property alias checked: toggle.checked

    property int fontSize: rpx(22)
    property int horizontalPadding: rpx(30)

    width: parent.width
    height: fontSize * 2.5


    Rectangle {
        id: underline

        width: parent.width
        height: rpx(3)
        anchors.bottom: parent.bottom

        color: "#3aa"
        visible: parent.focus
    }

    Text {
        id: label

        anchors.left: parent.left
        anchors.leftMargin: horizontalPadding
        anchors.verticalCenter: parent.verticalCenter

        color: "#eee"
        font.pixelSize: fontSize
        font.family: uiFont.name
    }

    Switch {
        id: toggle

        focus: true

        anchors.right: parent.right
        anchors.rightMargin: horizontalPadding
        anchors.verticalCenter: parent.verticalCenter

        height: fontSize * 1.1
    }
}
