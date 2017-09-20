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


import "gamepad" as GamepadConfig
import "settings"
import QtQuick 2.8


FocusScope {
    id: root

    signal close()
    signal requestShutdown()
    signal requestReboot()

    anchors.fill: parent
    visible: shade.opacity > 0

    Keys.onEscapePressed: if (!event.isAutoRepeat) stepBack()
    onFocusChanged: state = activeFocus ? "menuOpen" : ""


    // return to the main menu from a panel, or close the overlay
    function stepBack() {
        if (state == "menuOpen" || state == "") {
            close();
            return;
        }

        state = "menuOpen";
        menuPanel.enabled = true;
        menuPanel.focus = true;
    }

    // panel visibility is handled manually in order to
    // 1. control their visibility during transitions, and
    // 2. make sure only one of them is visible at a time (ie. no overlap)
    function hideAllSubpanels() {
        gamepadPanel.visible = false;
        settingsPanel.visible = false;
    }


    // capture right mouse button
    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.RightButton
        onClicked: root.stepBack()
    }

    // background
    Rectangle {
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
            text: pegasus.meta.gitRevision
            color: "#eee"
            font {
                pixelSize: rpx(12)
                family: "monospace"
            }
            anchors {
                left: parent.left
                bottom: parent.bottom
                margins: rpx(10)
            }
        }

        MouseArea {
            anchors.fill: parent
            onClicked: root.stepBack()
        }
    }

    MainMenuPanel {
        id: menuPanel
        focus: true
        anchors.left: parent.right

        onShowSettingsScreen: {
            parent.state = "settings";
            settingsPanel.visible = true;
        }
        onShowGamepadScreen: {
            parent.state = "gamepad";
            gamepadPanel.visible = true;
        }

        onRequestShutdown: root.requestShutdown()
        onRequestReboot: root.requestReboot()
    }

    SettingsPanel {
        id: settingsPanel
        anchors.left: menuPanel.right
        width: parent.width - menuPanel.width

        onScreenClosed: stepBack()
    }

    GamepadConfig.ConfigScreen {
        id: gamepadPanel
        anchors.left: menuPanel.right

        onScreenClosed: stepBack()
    }


    states: [
        State {
            // the shade should be visible in every state
            name: "_overlayOpen"
            PropertyChanges { target: shade; opacity: 0.75 }
        },
        State {
            name: "menuOpen"; extend: "_overlayOpen"
            PropertyChanges { target: revision; visible: true }
            AnchorChanges {
                target: menuPanel;
                anchors.left: undefined
                anchors.right: parent.right;
            }
        },
        State {
            // the main menu panel should not receive input
            // while any of the subpanel is open
            name: "_submenuOpen"; extend: "_overlayOpen"
            PropertyChanges {
                target: menuPanel
                enabled: false
            }
        },
        State {
            name: "_submenuWithPanel"; extend: "_submenuOpen"
            AnchorChanges {
                target: menuPanel;
                anchors.left: parent.left
                anchors.right: undefined
            }
        },
        State {
            name: "_submenuWithoutPanel"; extend: "_submenuOpen"
            AnchorChanges {
                target: menuPanel;
                anchors.left: undefined
                anchors.right: parent.left
            }
        },
        State {
            name: "settings"; extend: "_submenuWithPanel"
            PropertyChanges { target: settingsPanel; focus: true }
        },
        State {
            name: "gamepad"; extend: "_submenuWithoutPanel"
            PropertyChanges { target: gamepadPanel; focus: true }
        }
    ]


    // fancy easing curves, a la material design
    readonly property var bezierDecelerate: [ 0,0, 0.2,1, 1,1 ]
    readonly property var bezierSharp: [ 0.4,0, 0.6,1, 1,1 ]
    readonly property var bezierStandard: [ 0.4,0, 0.2,1, 1,1 ]

    transitions: [
        // main panel
        Transition {
            from: ""; to: "menuOpen"
            AnchorAnimation {
                duration: 225
                easing { type: Easing.Bezier; bezierCurve: bezierDecelerate }
            }
        },
        Transition {
            from: "menuOpen"; to: ""
            AnchorAnimation {
                duration: 200
                easing { type: Easing.Bezier; bezierCurve: bezierSharp }
            }
        },

        // non-fullscreen panels (shorter animations)
        Transition {
            from: "menuOpen"; to: "settings"
            AnchorAnimation {
                duration: 425
                easing { type: Easing.Bezier; bezierCurve: bezierStandard }
            }
        },
        Transition {
            from: "settings"; to: "menuOpen"
            AnchorAnimation {
                duration: 425
                easing { type: Easing.Bezier; bezierCurve: bezierStandard }
            }

            onRunningChanged: if (!running) hideAllSubpanels()
        },

        // full-screen panels (longer animations)
        Transition {
            from: "menuOpen"; to: "gamepad"
            AnchorAnimation {
                duration: 500
                easing { type: Easing.Bezier; bezierCurve: bezierStandard }
            }
        },
        Transition {
            from: "gamepad"; to: "menuOpen"
            AnchorAnimation {
                duration: 500
                easing { type: Easing.Bezier; bezierCurve: bezierStandard }
            }

            onRunningChanged: if (!running) hideAllSubpanels()
        }
    ]
}
