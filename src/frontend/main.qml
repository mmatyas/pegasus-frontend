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

// FIXME: without this include, QtGamepad is
// not included in the static build... Qt bug?
import QtGamepad 1.0


Window {
    id: appWindow
    visible: true
    width: 1280
    height: 720
    title: "Pegasus"
    color: "#000"

    visibility: api.settings.fullscreen
                ? Window.FullScreen : Window.AutomaticVisibility

    // provide relative pixel value calculation, for convenience
    property real winScale: Math.min(width / 1280.0, height / 720.0)
    function vpx(value) {
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
            Keys.onPressed: {
                if (event.key === Qt.Key_F5) {
                    event.accepted = true;

                    themeContent.source = "";
                    api.meta.clearQMLCache();
                    themeContent.source = Qt.binding(function() {
                        return api.settings.themes.current.qmlPath;
                    });
                }
            }

            source: api.settings.themes.current.qmlPath
            asynchronous: true
            onStatusChanged: {
                if (status == Loader.Error)
                    themeContent.source = "messages/ThemeError.qml";
                else if (status == Loader.Ready)
                    item.focus = true;
            }
        }

        Loader {
            id: mainMenu
            anchors.fill: parent
            sourceComponent: MainMenuOverlay { focus: true }
            asynchronous: true
        }
        Connections {
            target: mainMenu.item
            onClose: {
                themeContent.enabled = true;
                themeContent.focus = true;
            }
            onRequestShutdown: {
                content.enabled = false;
                powerDialog.source = "dialogs/ShutdownDialog.qml"
                powerDialog.focus = true;
            }
            onRequestReboot: {
                content.enabled = false;
                powerDialog.source = "dialogs/RebootDialog.qml"
                powerDialog.focus = true;
            }
            onRequestQuit: {
                themeContent.source = "";
                api.system.quit();
            }
        }
    }

    SplashScreen {
        id: splashScreen

        property bool loading: themeContent.status === Loader.Loading || api.meta.isLoading

        onLoadingChanged: {
            if (!loading) {
                if (api.collections.count === 0)
                    themeContent.source = "messages/NoGamesError.qml";

                // break bindings
                loading = false;
                visible = false;
                enabled = false;
                content.focus = true;
            }
        }
    }

    Loader {
        id: powerDialog
        anchors.fill: parent
    }
    Connections {
        target: powerDialog.item
        onCancel: {
            content.enabled = true;
            content.focus = true;
        }
    }
}
