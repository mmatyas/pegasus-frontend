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
    property alias note: sublabel.text
    property alias checked: toggle.checked

    property int fontSize: vpx(22)
    property int horizontalPadding: vpx(30)

    width: parent.width
    height: labelContainer.height + fontSize * 1.25


    Rectangle {
        id: underline

        width: parent.width
        height: vpx(3)
        anchors.bottom: parent.bottom

        color: "#3aa"
        visible: parent.focus
    }

    Column {
        id: labelContainer
        anchors {
            left: parent.left; leftMargin: horizontalPadding
            right: parent.horizontalCenter
            verticalCenter: parent.verticalCenter
        }

        spacing: fontSize * 0.25
        height: label.height + (sublabel.text ? spacing + sublabel.height : 0)


        Text {
            id: label

            color: "#eee"
            font.pixelSize: fontSize
            font.family: uiFont.name
        }

        Text {
            id: sublabel

            color: "#999"
            font.pixelSize: fontSize * 0.8
            font.family: uiFont.name
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
}
