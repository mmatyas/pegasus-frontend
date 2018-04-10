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

    readonly property int bodyTextSize: vpx(18)

    readonly property color headerColor: "#eee"
    readonly property color bodyColor: "#ccc"
    readonly property color instructionColor: "#fb5"


    Column {
        spacing: bodyTextSize * 1.5

        anchors {
            left: parent.left; leftMargin: vpx(50)
            right: parent.horizontalCenter
            verticalCenter: parent.verticalCenter
        }

        Text {
            text: qsTr("No games found :(") + api.tr

            width: parent.width
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignJustify

            color: headerColor
            font {
                bold: true
                pixelSize: vpx(50)
                family: globalFonts.sans
            }
        }

        Text {
            text: qsTr("Pegasus couldn't find any games on your device. If you have not"
                + " set up Pegasus yet, you can find the documentation here: <i>%1</i>."
                + "<br>"
                + "If you still see this message, make sure your config files are readable,"
                + " exist in one of the expected locations and are in the expected format.")
                    .arg("http://pegasus-frontend.org/docs")
                + api.tr

            width: parent.width
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignJustify

            color: bodyColor
            font.pixelSize: bodyTextSize
            font.family: globalFonts.sans
        }

        Text {
            text: qsTr("Please see the log file for more details.") + api.tr

            width: parent.width
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignJustify

            color: instructionColor
            font.pixelSize: vpx(20)
            font.family: globalFonts.sans
        }

        Text {
            text: qsTr("You can find it here:<pre>%1</pre>")
                    .arg(api.meta.logFilePath)
                + api.tr

            width: parent.width
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignJustify

            color: bodyColor
            font.pixelSize: bodyTextSize
            font.family: globalFonts.sans
        }
    }
}
