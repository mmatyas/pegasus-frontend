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


import "menuitems"
import QtQuick 2.8


FocusScope {
    Keys.onEscapePressed: {
        themeContent.enabled = true
        themeContent.focus = true
    }


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
        visible: opacity > 0
    }

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
        visible: false
    }

    Rectangle {
        id: menuPanel
        color: "#333"
        width: rpx(420)
        height: parent.height
        visible: x < parent.width
        x: parent.width

        property int bottomPadding: rpx(30)


        Column {
            width: parent.width
            z: 200
            anchors {
                bottom: parent.bottom
                bottomMargin: {
                    if (quitSubmenu.focus) return quitSubmenu.height + parent.bottomPadding;
                    return parent.bottomPadding;
                }
            }

            Behavior on anchors.bottomMargin {
                PropertyAnimation {
                    id: slideAnim
                    duration: 150
                }
            }

            PrimaryMenuItem {
                id: mbSettings
                text: qsTr("Settings")
                selected: focus

                focus: true
                KeyNavigation.down: mbControls
            }
            PrimaryMenuItem {
                id: mbControls
                text: qsTr("Controls")
                selected: focus

                KeyNavigation.up: mbSettings
                KeyNavigation.down: mbQuit
            }
            PrimaryMenuItem {
                id: mbQuit
                text: qsTr("Quit")
                onActivated: {
                    if (quitSubmenu.focus) mbQuit.forceActiveFocus()
                    else quitSubmenu.forceActiveFocus()
                }
                selected: focus || quitSubmenu.visible

                KeyNavigation.up: mbControls
            }
        }

        FocusScope {
            id: quitSubmenu
            width: parent.width
            height: quitSubmenuColumn.height
            visible: focus || (mbQuit.focus && slideAnim.running)
            anchors {
                bottom: parent.bottom
                bottomMargin: parent.bottomPadding
            }

            Column {
                id: quitSubmenuColumn
                width: parent.width
                z: 100

                SecondaryMenuItem {
                    id: mbQuitShutdown
                    text: qsTr("Shutdown")
                    onActivated: pegasus.system.shutdown()

                    focus: true
                    KeyNavigation.down: mbQuitReboot
                    Keys.onEscapePressed: mbQuit.forceActiveFocus()
                }
                SecondaryMenuItem {
                    id: mbQuitReboot
                    text: qsTr("Reboot")
                    onActivated: pegasus.system.reboot()

                    KeyNavigation.up: mbQuitShutdown
                    KeyNavigation.down: mbQuitExit
                    Keys.onEscapePressed: mbQuit.forceActiveFocus()
                }
                SecondaryMenuItem {
                    id: mbQuitExit
                    text: qsTr("Exit Pegasus")
                    onActivated: Qt.quit()

                    KeyNavigation.up: mbQuitReboot
                    Keys.onEscapePressed: mbQuit.forceActiveFocus()
                }
            }
        }
    }

    states: State {
        name: "menuOpen"; when: activeFocus
        PropertyChanges { target: shade; opacity: 0.75 }
        PropertyChanges { target: revision; visible: true }
        PropertyChanges { target: menuPanel; x: parent.width - width }
    }

    transitions: Transition {
        NumberAnimation { properties: "opacity,x"; duration: 300; easing.type: Easing.InOutCubic }
    }
}
