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
import QtQuick.Window 2.2
import QtGamepad 1.0

Window {
    id: appWindow
    visible: true
    width: 1280
    height: 720
    title: "Pegasus"
    color: "#000"

    // provide relative pixel value calculation, for convenience
    property real winScale: Math.min(width / 1280.0, height / 720.0)
    function rpx(value) {
        return winScale * value;
    }

    // register custom global fonts
    FontLoader { id: uiFont; source: "/fonts/Roboto.ttf" }
    FontLoader { id: uiFontCondensed; source: "/fonts/RobotoCondensed.ttf" }


    function toggleMenu() {
        if (themeContent.focus) {
            themeContent.enabled = false;
            mainMenu.focus = true;
        }
        else {
            themeContent.enabled = true;
            themeContent.focus = true;
        }
    }

    FocusScope {
        focus: !splashScreen.visible
        anchors.fill: parent

        Keys.onEscapePressed: toggleMenu()

        Loader {
            id: themeContent
            anchors.fill: parent
            focus: true

            source: pegasus.settings.themes.current.qmlPath
            asynchronous: true
        }

        MainMenuOverlay {
            id: mainMenu
            anchors.fill: parent
        }
    }

    SplashScreen {
        id: splashScreen
        visible: (themeContent.status != Loader.Ready) || pegasus.meta.isLoading
    }
}
