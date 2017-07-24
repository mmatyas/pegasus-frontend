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


import QtQuick 2.8

FocusScope {
    signal screenClosed()

    height: parent.height

    Keys.onEscapePressed: screenClosed()

    Rectangle {
        anchors.fill: parent
        color: "#222"
    }

    Column {
        anchors {
            fill: parent
            margins: rpx(40)
            leftMargin: rpx(120)
        }
        spacing: rpx(20)

        HorizontalSelector {
            focus: true
            label: qsTr("Language") + pegasus.tr
            model: pegasus.settings.allLanguages
            index: pegasus.settings.languageIndex
            onIndexChanged: pegasus.settings.languageIndex = index

            KeyNavigation.down: setTheme
        }
        HorizontalSelector {
            id: setTheme
            label: qsTr("Theme") + pegasus.tr
            model: pegasus.settings.allThemes
            index: pegasus.settings.themeIndex
            onIndexChanged: pegasus.settings.themeIndex = index
        }
    }
}
