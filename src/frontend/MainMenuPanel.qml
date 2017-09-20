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
    id: root

    width: rpx(350)
    height: parent.height
    visible: x < parent.width && 0 < x + width

    signal showGamepadScreen()
    signal showSettingsScreen()

    signal requestShutdown()
    signal requestReboot()


    Rectangle {
        color: "#333"
        anchors.fill: parent

        Item {
            id: menuHeader
            width: parent.width
            height: rpx(30)
            anchors.top: parent.top
        }
        PrimaryMenuItem {
            id: mbSettings
            text: qsTr("Settings") + pegasus.tr
            onActivated: root.showSettingsScreen()
            selected: focus

            focus: true
            KeyNavigation.down: mbControls
            anchors.bottom: mbControls.top
        }
        PrimaryMenuItem {
            id: mbControls
            text: qsTr("Controls") + pegasus.tr
            onActivated: root.showGamepadScreen()
            selected: focus

            KeyNavigation.up: mbSettings
            KeyNavigation.down: mbQuit
            anchors.bottom: mbQuit.top
        }
        PrimaryMenuItem {
            id: mbQuit
            text: qsTr("Quit") + pegasus.tr
            onActivated: {
                if (quitSubmenu.focus) mbQuit.forceActiveFocus()
                else quitSubmenu.forceActiveFocus()
            }
            selected: focus || quitSubmenu.visible

            KeyNavigation.up: mbControls
            anchors.bottom: menuFooter.top
        }
        FocusScope {
            id: quitSubmenu
            width: parent.width
            height: quitSubmenuColumn.height
            visible: focus || (mbQuit.focus && submenuAnim.running)
            anchors.bottom: menuFooter.top

            Column {
                id: quitSubmenuColumn
                width: parent.width

                SecondaryMenuItem {
                    id: mbQuitShutdown
                    text: qsTr("Shutdown") + pegasus.tr
                    onActivated: requestShutdown()

                    focus: true
                    KeyNavigation.down: mbQuitReboot
                    Keys.onEscapePressed: mbQuit.forceActiveFocus()
                }
                SecondaryMenuItem {
                    id: mbQuitReboot
                    text: qsTr("Reboot") + pegasus.tr
                    onActivated: requestReboot()

                    KeyNavigation.up: mbQuitShutdown
                    KeyNavigation.down: mbQuitExit
                    Keys.onEscapePressed: mbQuit.forceActiveFocus()
                }
                SecondaryMenuItem {
                    id: mbQuitExit
                    text: qsTr("Exit Pegasus") + pegasus.tr
                    onActivated: pegasus.system.quit()

                    KeyNavigation.up: mbQuitReboot
                    Keys.onEscapePressed: mbQuit.forceActiveFocus()
                }
            }
        }
        Item {
            id: menuFooter
            width: parent.width
            height: rpx(30)
            anchors.bottom: parent.bottom
        }

        states: [
            State {
                name: "quitOpen"; when: quitSubmenu.focus
                AnchorChanges { target: mbQuit; anchors.bottom: quitSubmenu.top }
            }
        ]

        transitions: Transition {
            id: submenuAnim
            AnchorAnimation { duration: 150 }
        }
    }
}
