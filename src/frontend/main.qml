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


import QtQuick 2.6
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

    // register custom global fonts here
    FontLoader { id: font_loading; source: "/fonts/loading.ttf" }

    // enable gamepad support
    Gamepad {
        id: gamepad1
        deviceId: GamepadManager.connectedGamepads.length > 0
                  ? GamepadManager.connectedGamepads[0]
                  : -1
    }
    Connections {
        target: GamepadManager
        onGamepadConnected: gamepad1.deviceId = deviceId
    }
    GamepadKeyNavigation {
        id: gamepadKeyNav
        gamepad: gamepad1
        active: true
        buttonAKey: Qt.Key_Return
        buttonL1Key: Qt.Key_A
        buttonL2Key: Qt.Key_A
        buttonR1Key: Qt.Key_D
        buttonR2Key: Qt.Key_D
    }


    FocusScope {
        focus: !loadingScreen.visible
        anchors.fill: parent

        Keys.onEscapePressed: {
            themeContent.enabled = false
            mainMenu.focus = true
        }

        Loader {
            id: themeContent
            anchors.fill: parent
            focus: true

            source: "/themes/pegasus-grid/theme.qml"
            asynchronous: true
        }

        MainMenu {
            id: mainMenu
            anchors.fill: parent
        }
    }


    Rectangle {
        id: loadingScreen
        color: "#222"
        anchors.fill: parent

        visible: (themeContent.status != Loader.Ready) || pegasus.meta.isLoading

        Text {
            text: "PEGASUS"
            color: "#333"
            font {
                pixelSize: rpx(280)
                family: font_loading.name
            }
            anchors {
                bottom: parent.bottom
                left: parent.left
                leftMargin: rpx(56)
            }
        }
    }
}
