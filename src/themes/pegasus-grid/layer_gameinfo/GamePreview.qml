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


import QtQuick 2.0
import "qrc:/qmlutils" as PegasusUtils


FocusScope {
    id: root

    property alias panelWidth: panelRect.width
    property bool renderLeftside: false

    signal openRequested
    signal closeRequested
    signal filtersRequested

    visible: api.currentGame

    Keys.onPressed: {
        if (event.isAutoRepeat)
            return;

        if (api.keys.isCancel(event.key) || api.keys.isDetails(event.key)) {
            event.accepted = true;
            closeRequested();
            return;
        }
        if (api.keys.isFilters(event.key)) {
            event.accepted = true;
            filtersRequested();
            return;
        }
    }

    Rectangle {
        id: panelRect
        color: "#111"
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.left
        opacity: 0.85

        PegasusUtils.HorizontalSwipeArea {
            anchors.fill: parent

            onSwipeRight: openRequested()
            onSwipeLeft: closeRequested()
            onClicked: mouse.accepted = true
        }

        Rectangle {
            id: panelBorder
            color: "#222"
            width: vpx(2)
            height: parent.height
            anchors.left: parent.right
        }

        PanelLeft {
            id: leftColumn
            anchors {
                left: parent.left; leftMargin: vpx(16)
                right: parent.horizontalCenter; rightMargin: vpx(20)
                top: parent.top; topMargin: vpx(40)
                bottom: parent.bottom; bottomMargin: vpx(8)
            }

            visible: root.renderLeftside
        }

        PanelRight {
            id: rightside
            anchors {
                left: parent.horizontalCenter; leftMargin: vpx(20)
                right: parent.right; rightMargin: vpx(20)
                top: parent.top; topMargin: vpx(40)
                bottom: parent.bottom; bottomMargin: vpx(8)
            }
        }
    }

    MouseArea {
        anchors {
            left: panelRect.right; right: parent.right
            top: parent.top; bottom: parent.bottom
        }
        enabled: root.focus
        onClicked: closeRequested()
    }

    states: [
        State {
            name: "open"; when: root.focus
            AnchorChanges {
                target: panelRect
                anchors.left: parent.left
                anchors.horizontalCenter: undefined
            }
        }
    ]
    transitions: [
        Transition {
            to: "open"
            onRunningChanged: {
                if (running)
                    root.renderLeftside = true;
            }
            AnchorAnimation { duration: 300; easing.type: Easing.OutCubic }
        },
        Transition {
            from: "open"
            onRunningChanged: {
                if (!running)
                    root.renderLeftside = false;
            }
            AnchorAnimation { duration: 300; easing.type: Easing.OutCubic }
        }
    ]
}
