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


Rectangle {
    anchors.fill: parent
    color: "#000"

    property alias title: txtTitle.text
    property alias details: txtDescription.text
    property alias instruction: txtInstruction.text
    property alias logInfo: txtLogInfo.text

    readonly property int titleTextSize: vpx(50)
    readonly property int detailsTextSize: vpx(18)
    readonly property int instructionTextSize: vpx(20)

    readonly property color titleColor: "#eee"
    readonly property color detailsColor: "#ccc"
    readonly property color instructionColor: "#fb5"


    Column {
        spacing: detailsTextSize * 1.5

        anchors {
            left: parent.left; leftMargin: vpx(50)
            right: parent.horizontalCenter
            verticalCenter: parent.verticalCenter
        }


        Text {
            id: txtTitle

            color: titleColor
            font.bold: true
            font.pixelSize: titleTextSize
            font.family: globalFonts.sans
        }

        Text {
            id: txtDescription

            width: parent.width
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignJustify

            color: detailsColor
            font.pixelSize: detailsTextSize
            font.family: globalFonts.sans
        }

        Text {
            id: txtInstruction

            width: parent.width
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignJustify

            color: instructionColor
            font.pixelSize: instructionTextSize
            font.family: globalFonts.sans
        }

        Text {
            id: txtLogInfo

            width: parent.width
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignJustify

            color: detailsColor
            font.pixelSize: detailsTextSize
            font.family: globalFonts.sans
        }
    }
}
