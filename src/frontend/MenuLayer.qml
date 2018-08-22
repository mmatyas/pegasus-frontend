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


import "menu"
import QtQuick 2.0


FocusScope {
    id: root

    function open() {
        root.state = "menu";
    }

    signal close()
    signal requestShutdown()
    signal requestReboot()
    signal requestQuit()

    function triggerClose() {
        root.state = "";
        root.close();
    }

    anchors.fill: parent
    visible: shade.opacity > 0

    Keys.onEscapePressed: triggerClose()


    // capture right mouse button
    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.RightButton
        onClicked: root.triggerClose()
    }

    Rectangle {
        // background
        id: shade
        anchors {
            left: parent.left
            top: parent.top
            bottom: parent.bottom
            right: menuPanel.left
        }

        color: "black"
        opacity: 0
        visible: opacity > 0 && width > 0

        Behavior on opacity { NumberAnimation { duration: 300 } }

        Text {
            id: revision
            text: api.meta.gitRevision + ", " + api.meta.gitDate
            color: "#eee"
            font {
                pixelSize: vpx(12)
                family: "monospace"
            }
            anchors {
                left: parent.left
                bottom: parent.bottom
                margins: vpx(10)
            }
        }

        MouseArea {
            // capture left mouse button
            anchors.fill: parent
            onClicked: root.triggerClose()
        }
    }

    MainMenuPanel {
        id: menuPanel
        anchors.left: parent.right

        function openScreen(url) {
            subscreen.source = url;
            root.state = "sub";
        }

        onShowSettingsScreen: openScreen("menu/SettingsScreen.qml")
        onShowGamepadScreen: openScreen("menu/GamepadScreen.qml")
        onShowHelpScreen: openScreen("menu/HelpScreen.qml")

        onClose: root.triggerClose()
        onRequestShutdown: root.requestShutdown()
        onRequestReboot: root.requestReboot()
        onRequestQuit: root.requestQuit()
    }

    Loader {
        id: subscreen
        anchors.left: menuPanel.right

        width: parent.width
        height: parent.height

        enabled: false
    }
    Connections {
        target: subscreen.item
        onClose: root.state = "menu"
    }


    states: [
        State {
            // the shade should be visible in every state
            name: "_shade"
            PropertyChanges {
                target: shade
                opacity: 0.75
            }
        },
        State {
            name: "menu"; extend: "_shade"
            PropertyChanges {
                target: menuPanel
                focus: true
                enabled: true
            }
            AnchorChanges {
                target: menuPanel;
                anchors.left: undefined
                anchors.right: parent.right;
            }
        },
        State {
            name: "sub"; extend: "_shade"
            PropertyChanges {
                target: menuPanel
                enabled: false
            }
            PropertyChanges {
                target: subscreen
                enabled: true
                focus: true
            }
            AnchorChanges {
                target: menuPanel;
                anchors.left: undefined
                anchors.right: parent.left
            }
        }
    ]

    // fancy easing curves, a la material design
    readonly property var bezierDecelerate: [ 0,0, 0.2,1, 1,1 ]
    readonly property var bezierSharp: [ 0.4,0, 0.6,1, 1,1 ]
    readonly property var bezierStandard: [ 0.4,0, 0.2,1, 1,1 ]

    transitions: [
        Transition {
            from: ""; to: "menu"
            AnchorAnimation {
                duration: 225
                easing { type: Easing.Bezier; bezierCurve: bezierDecelerate }
            }
        },
        Transition {
            from: "menu"; to: ""
            AnchorAnimation {
                duration: 200
                easing { type: Easing.Bezier; bezierCurve: bezierSharp }
            }
            onRunningChanged: if (!running) subscreen.source = ""
        },
        Transition {
            from: "menu"; to: "sub"
            AnchorAnimation {
                duration: 425
                easing { type: Easing.Bezier; bezierCurve: bezierStandard }
            }
        },
        Transition {
            from: "sub"; to: "menu"
            AnchorAnimation {
                duration: 425
                easing { type: Easing.Bezier; bezierCurve: bezierStandard }
            }
        }
    ]
}
