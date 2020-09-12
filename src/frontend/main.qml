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


import "qrc:/qmlutils" as PegasusUtils
import QtQuick 2.8
import QtQuick.Window 2.2


Window {
    id: appWindow
    visible: true
    width: 1280
    height: 720
    title: "Pegasus"
    color: "#000"

    visibility: api.internal.settings.fullscreen
                ? Window.FullScreen : Window.AutomaticVisibility

    onClosing: {
        theme.source = "";
        api.internal.system.quit();
    }

    FontLoader { id: sansFont; source: "/fonts/Roboto.ttf" }
    FontLoader { id: condensedFont; source: "/fonts/RobotoCondensed.ttf" }


    // a globally avalable utility object
    QtObject {
        id: global

        readonly property real winScale: Math.min(width / 1280.0, height / 720.0)

        readonly property var fonts: QtObject {
            readonly property string sans: sansFont.name
            readonly property string condensed: condensedFont.name
        }
    }


    // legacy global objects
    QtObject {
        id: globalFonts
        readonly property string sans: global.fonts.sans
        readonly property string condensed: global.fonts.condensed
    }
    function vpx(value) {
        return global.winScale * value;
    }


    // the main content
    FocusScope {
        id: content
        anchors.fill: parent
        enabled: focus

        Loader {
            id: theme
            anchors.fill: parent

            focus: true
            enabled: focus

            readonly property url apiThemePath: api.internal.settings.themes.currentQmlPath

            function getThemeFile() {
                if (api.internal.meta.isLoading)
                    return "";
                if (api.collections.count === 0)
                    return "messages/NoGamesError.qml";

                return apiThemePath;
            }
            onApiThemePathChanged: source = Qt.binding(getThemeFile)

            Keys.onPressed: {
                if (api.keys.isCancel(event) || api.keys.isMenu(event)) {
                    event.accepted = true;
                    mainMenu.focus = true;
                }

                if (event.key === Qt.Key_F5) {
                    event.accepted = true;

                    theme.source = "";
                    api.internal.meta.clearQMLCache();
                    theme.source = Qt.binding(getThemeFile);
                }
            }

            source: getThemeFile()
            asynchronous: true
            onStatusChanged: {
                if (status == Loader.Error)
                    source = "messages/ThemeError.qml";
            }
            onLoaded: item.focus = focus
            onFocusChanged: if (item) item.focus = focus
        }

        Loader {
            id: mainMenu
            anchors.fill: parent

            source: "MenuLayer.qml"
            asynchronous: true

            onLoaded: item.focus = focus
            onFocusChanged: if (item) item.focus = focus
            enabled: focus
        }
        Connections {
            target: mainMenu.item

            function onClose() { theme.focus = true; }

            function onRequestShutdown() {
                powerDialog.source = "dialogs/ShutdownDialog.qml"
                powerDialog.focus = true;
            }
            function onRequestReboot() {
                powerDialog.source = "dialogs/RebootDialog.qml"
                powerDialog.focus = true;
            }
            function onRequestQuit() {
                theme.source = "";
                api.internal.system.quit();
            }
        }
        PegasusUtils.HorizontalSwipeArea {
            id: menuSwipe

            width: vpx(40)
            height: parent.height
            anchors.right: parent.right

            onSwipeLeft: {
                if (!mainMenu.focus)
                    mainMenu.focus = true;
            }
        }
    }


    Loader {
        id: powerDialog
        anchors.fill: parent
    }
    Connections {
        target: powerDialog.item
        function onCancel() { content.focus = true; }
    }


    Loader {
        id: multifileSelector
        anchors.fill: parent
    }
    Connections {
        target: multifileSelector.item
        function onCancel() { content.focus = true; }
    }


    Loader {
        id: genericMessage
        anchors.fill: parent
    }
    Connections {
        target: genericMessage.item
        function onClose() { content.focus = true; }
    }


    Connections {
        target: api
        function onEventSelectGameFile(game) {
            multifileSelector.setSource("dialogs/MultifileSelector.qml", {"game": game})
            multifileSelector.focus = true;
        }
        function onEventLaunchError(msg) {
            genericMessage.setSource("dialogs/GenericOkDialog.qml",
                { "title": qsTr("Error"), "message": msg });
            genericMessage.focus = true;
        }
        function onEventLoadingStarted() {
            splashScreen.focus = true;
        }
    }


    SplashLayer {
        id: splashScreen
        focus: true
        enabled: false
        visible: focus

        property bool dataLoading: api.internal.meta.loading
        property bool skinLoading: theme.status === Loader.Null || theme.status === Loader.Loading
        showDataProgressText: dataLoading

        function hideMaybe() {
            if (focus && !dataLoading && !skinLoading) {
                content.focus = true;
                api.internal.meta.resetLoadingState();
            }
        }

        onSkinLoadingChanged: hideMaybe()
        onDataLoadingChanged: hideMaybe()
    }
}
