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
    Keys.onPressed: {
        if (event.isAutoRepeat)
            return;

        switch (event.key) {
            // game data
            case Qt.Key_Control:
                if (gamepreview.focus)
                    gamegrid.focus = true;
                else if (gamegrid.focus)
                    gamepreview.focus = true;
                break;
            // filtering
            case Qt.Key_Shift:
                if (filter.focus)
                    content.focus = true;
                else if (gamegrid.focus)
                    filter.focus = true;
                break;
            default:
                return;
        }

        event.accepted = true;
    }

    FocusScope {
        id: content
        anchors.fill: parent
        focus: true

        Keys.onPressed: {
            if (event.isAutoRepeat || event.modifiers)
                return;

            switch (event.key) {
                // platform bar -- QWERTx/AZERTY support
                case Qt.Key_Q:
                case Qt.Key_A:
                    topbar.prev();
                    break;
                case Qt.Key_E:
                case Qt.Key_D:
                    topbar.next();
                    break;
                default:
                    return;
            }

            event.accepted = true;
        }

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
            focus: true

            onLaunchRequested: api.currentGame.launch()
            onDetailsRequested: gamepreview.focus = true

            id: gamegrid
            width: (parent.width * 0.6) - vpx(32)
            anchors {
                top: topbar.bottom; topMargin: vpx(32)
                right: parent.right; rightMargin: vpx(6)
                bottom: parent.bottom
            }
        }

        GamePreview {
            id: gamepreview
            width: parent.width * 0.7 + vpx(72)
            anchors.top: topbar.bottom
            anchors.horizontalCenter: parent.left
            anchors.bottom: parent.bottom

            drawLeft: false
            onOpenRequested: gamepreview.focus = true
            onCloseRequested: gamegrid.focus = true
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
        },
        State {
            name: "gamedataOpen"; when: gamepreview.focus
            AnchorChanges {
                target: gamepreview
                anchors.left: parent.left
                anchors.horizontalCenter: undefined
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
        },
        Transition {
            to: "gamedataOpen"
            onRunningChanged: {
                if (running)
                    gamepreview.drawLeft = true;
            }
            AnchorAnimation { duration: 300; easing.type: Easing.OutCubic }
        },
        Transition {
            from: "gamedataOpen"
            onRunningChanged: {
                if (!running)
                    gamepreview.drawLeft = false;
            }
            AnchorAnimation { duration: 300; easing.type: Easing.OutCubic }
        }
    ]
}
