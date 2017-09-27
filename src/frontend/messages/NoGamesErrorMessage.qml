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


Rectangle {
    anchors.fill: parent
    color: "#000"

    readonly property int bodyTextSize: rpx(18)

    readonly property color headerColor: "#eee"
    readonly property color bodyColor: "#ccc"
    readonly property color instructionColor: "#fb5"


    Column {
        spacing: bodyTextSize * 1.5

        anchors {
            left: parent.left; leftMargin: rpx(50)
            right: parent.horizontalCenter
            verticalCenter: parent.verticalCenter
        }

        Text {
            text: qsTr("No games found :(") + pegasus.tr

            width: parent.width
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignJustify

            color: headerColor
            font {
                bold: true
                pixelSize: rpx(50)
                family: uiFont.name
            }
        }

        Text {
            text: qsTr("Pegasus couldn't find any games. At the moment,"
                + " the configuration and game list files of EmulationStation 2"
                + " are used. If you see this message, then these files are"
                + " either missing, not in the expected location, or contain"
                + " errors.")
                + pegasus.tr

            width: parent.width
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignJustify

            color: bodyColor
            font.pixelSize: bodyTextSize
            font.family: uiFont.name
        }

        Text {
            text: qsTr("Please see the log file for more details.") + pegasus.tr

            width: parent.width
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignJustify

            color: instructionColor
            font.pixelSize: rpx(20)
            font.family: uiFont.name
        }

        Text {
            text: qsTr("You can find it here:<pre>%1</pre>")
                    .arg(pegasus.meta.logFilePath)
                 + pegasus.tr

            width: parent.width
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignJustify

            color: bodyColor
            font.pixelSize: bodyTextSize
            font.family: uiFont.name
        }
    }
}
