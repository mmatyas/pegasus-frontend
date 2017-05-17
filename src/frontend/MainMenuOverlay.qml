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
    Keys.onEscapePressed: toggleMenu()

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

        Behavior on opacity { NumberAnimation { duration: 300 } }

        MouseArea {
            anchors.fill: parent
            onClicked: toggleMenu()
        }
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

    MainMenuPanel {
        id: menuPanel
        focus: true
        anchors.left: parent.right
    }

    states: State {
        name: "menuOpen"; when: activeFocus
        PropertyChanges { target: shade; opacity: 0.75 }
        PropertyChanges { target: revision; visible: true }
        AnchorChanges {
            target: menuPanel;
            anchors.right: parent.right;
            anchors.left: undefined
        }
    }

    transitions: Transition {
        AnchorAnimation { duration: 300; easing.type: Easing.OutCubic }
    }
}
