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

    property int fontSize
    property alias text: textInput.text
    property alias textColor: textInput.color
    property alias placeholder: placeholder.text
    property alias placeholderColor: placeholder.color

    height: fontSize * 1.7
    clip: true

    Rectangle {
        anchors.fill: parent

        color: "#50000000"
        border.color: "#60000000"
        border.width: vpx(1)
    }

    TextInput {
        id: textInput

        focus: true

        anchors.fill: parent
        anchors.leftMargin: vpx(8)
        anchors.rightMargin: vpx(8)

        verticalAlignment: Text.AlignVCenter

        color: "#eee"
        font {
            pixelSize: root.fontSize
            family: globalFonts.sans
        }
    }

    Text {
        id: placeholder
        anchors.fill: parent
        anchors.leftMargin: vpx(8)
        anchors.rightMargin: vpx(8)

        verticalAlignment: Text.AlignVCenter

        color: "#bbb"
        font {
            pixelSize: root.fontSize
            family: globalFonts.sans
            italic: true
        }

        visible: !textInput.text
    }
}
