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


    // the main content
    FocusScope {
        id: content
        anchors.fill: parent

        Loader {
            id: themeContent
            anchors.fill: parent

            focus: true
            Keys.onEscapePressed: {
                themeContent.enabled = false;
                mainMenu.focus = true;
            }

            source: pegasus.settings.themes.current.qmlPath
            asynchronous: true
        }

        MainMenuOverlay {
            id: mainMenu

            onClose: {
                themeContent.enabled = true;
                themeContent.focus = true;
            }
            onRequestShutdown: {
                content.enabled = false;
                shutdownDialog.focus = true;
            }
            onRequestReboot: {
                content.enabled = false;
                rebootDialog.focus = true;
            }
        }
    }

    SplashScreen {
        id: splashScreen

        visible: (themeContent.status != Loader.Ready) || pegasus.meta.isLoading
        onVisibleChanged: {
            visible = false; // break binding
            content.focus = true;
        }
    }

    DialogOkCancel {
        id: shutdownDialog

        title: qsTr("Shutdown") + pegasus.tr
        message: qsTr("The system will shut down. Are you sure?") + pegasus.tr
        symbol: "\u23FB"

        onAccept: pegasus.system.shutdown()
        onCancel: {
            content.enabled = true;
            content.focus = true;
        }
    }

    DialogOkCancel {
        id: rebootDialog

        title: qsTr("Reboot") + pegasus.tr
        message: qsTr("The system will reboot. Are you sure?") + pegasus.tr
        symbol: "\u21BB"

        onAccept: pegasus.system.reboot()
        onCancel: {
            content.enabled = true;
            content.focus = true;
        }
    }
}
