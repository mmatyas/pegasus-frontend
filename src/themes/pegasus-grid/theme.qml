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
import "filter_panel"


FocusScope {
    Keys.onReleased: if (event.key === Qt.Key_Control && !event.isAutoRepeat) {
        if (filter.focus)
            content.focus = true
        else
            filter.focus = true
    }

    Item {
        id: content

        focus: true
        Keys.forwardTo: [topbar, gamegrid]
        Keys.onReturnPressed: api.currentGame.launch()

        anchors.fill: parent

        PlatformBar {
            id: topbar
            z: 300
            width: parent.width
        }

        BackgroundImage {
            anchors {
                top: topbar.bottom; bottom: parent.bottom
                left: parent.left; right: parent.right
            }
        }

        GameGrid {
            id: gamegrid
            z: 100
            width: (parent.width * 0.6) - vpx(32)
            anchors {
                top: topbar.bottom; topMargin: vpx(32)
                right: parent.right; rightMargin: vpx(6)
                bottom: parent.bottom
            }
        }

        GamePreview {
            z: 200
            width: parent.width * 0.35 + vpx(48)
            anchors {
                top: topbar.bottom
                left: parent.left
                bottom: parent.bottom
            }
        }
    }

    FocusScope {
        id: filter
        anchors.fill: parent

        Rectangle {
            id: filterShade
            anchors.fill: parent
            color: "black"
            opacity: 0

            Behavior on opacity { NumberAnimation { duration: 500 } }
        }

        FilterPanel {
            id: filterPanel
            z: 400
            focus: true
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.left
            visible: false
        }
    }

    states: [
        State {
            name: "filtersOpen"; when: filter.focus
            PropertyChanges { target: filterShade; opacity: 0.2 }
            AnchorChanges {
                target: filterPanel
                anchors.left: parent.left
                anchors.right: undefined
            }
        }
    ]

    transitions: [
        Transition {
            to: "filtersOpen"
            onRunningChanged: {
                if (running)
                    filterPanel.visible = true;
            }
            AnchorAnimation { duration: 500; easing.type: Easing.OutCubic }
        },
        Transition {
            from: "filtersOpen"
            onRunningChanged: {
                if (!running)
                    filterPanel.visible = false;
            }
            AnchorAnimation { duration: 300; easing.type: Easing.OutCubic }
        }
    ]
}
