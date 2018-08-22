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


import QtQuick 2.6

Column {
    readonly property int fontSize: vpx(30)

    property alias text: title.text

    width: parent.width


    Text {
        id: title

        color: "#eee"
        font.pixelSize: fontSize
        font.family: globalFonts.sans
        font.bold: true

        width: parent.width
        topPadding: fontSize
        bottomPadding: fontSize * 0.25
    }

    Rectangle {
        width: parent.width
        height: vpx(2)
        color: "#555"
    }
}
