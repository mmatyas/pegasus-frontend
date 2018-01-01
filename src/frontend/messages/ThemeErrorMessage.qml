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
            text: qsTr("Theme loading failed :(") + pegasus.tr

            width: parent.width
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignJustify

            color: headerColor
            font {
                bold: true
                pixelSize: vpx(50)
                family: uiFont.name
            }
        }

        Text {
            text: qsTr("Pegasus tried to load the selected theme (%1), but failed."
                + " This may happen when you try to load an outdated theme, or when"
                + " there's a bug in its code.")
                    .arg(pegasus.settings.themes.current.name)
                + pegasus.tr

            width: parent.width
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignJustify

            color: bodyColor
            font.pixelSize: bodyTextSize
            font.family: uiFont.name
        }

        Text {
            text: qsTr("For now, please select a different theme from the main menu.") + pegasus.tr

            width: parent.width
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignJustify

            color: instructionColor
            font.pixelSize: vpx(20)
            font.family: uiFont.name
        }

        Text {
            text: qsTr("You might find more details in the log file:<pre>%1</pre>")
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
