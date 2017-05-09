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
    Keys.onEscapePressed: themeContent.focus = true


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

    Rectangle {
        id: menuPanel
        color: "#333"
        width: rpx(420)
        height: parent.height
        visible: x < parent.width
        x: parent.width
    }

    states: State {
        name: "menuOpen"; when: activeFocus
        PropertyChanges { target: shade; opacity: 0.75 }
        PropertyChanges { target: menuPanel; x: parent.width - width }
    }

    transitions: Transition {
        NumberAnimation { properties: "opacity,x"; duration: 300; easing.type: Easing.InOutCubic }
    }
}
