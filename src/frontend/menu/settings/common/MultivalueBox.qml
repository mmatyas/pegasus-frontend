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


import QtQuick 2.6


FocusScope {
    id: root

    property alias model: list.model
    property alias index: list.currentIndex

    readonly property int textSize: vpx(22)
    readonly property int itemHeight: 2.25 * textSize

    signal close
    signal select(int index)

    onFocusChanged: if (focus) root.state = "open";
    function triggerClose() {
        root.state = "";
        root.close();
    }


    anchors.fill: parent
    enabled: focus
    visible: focus || animClosing.running

    Keys.onPressed: {
        if (event.isAutoRepeat)
            return;

        if (api.keys.isCancel(event)) {
            event.accepted = true;
            triggerClose();
        }
        else if (api.keys.isAccept(event)) {
            event.accepted = true;
            select(index);
            triggerClose();
        }
    }

    Component.onCompleted: {
        if (list.currentIndex > 0)
            list.positionViewAtIndex(list.currentIndex, ListView.Center);
    }


    Rectangle {
        id: shade

        anchors.fill: parent
        color: "#000"

        opacity: parent.focus ? 0.3 : 0.0
        Behavior on opacity { PropertyAnimation { duration: 150 } }

        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.LeftButton | Qt.RightButton
            onClicked: root.triggerClose()
        }
    }


    Rectangle {
        id: box

        width: vpx(280)
        height: parent.height * 0.84
        anchors.verticalCenter: parent.verticalCenter

        anchors.left: parent.right
        anchors.rightMargin: height * 0.04
        visible: x < parent.width

        color: "#eee"
        radius: vpx(8)

        MouseArea {
            anchors.fill: parent
        }

        Item {
            anchors.fill: parent
            anchors.topMargin: parent.radius
            anchors.bottomMargin: parent.radius
            clip: true

            ListView {
                id: list
                focus: true

                width: parent.width
                height: Math.min(count * itemHeight, parent.height)
                anchors.verticalCenter: parent.verticalCenter

                delegate: listItem

                snapMode: ListView.SnapOneItem
                highlightMoveDuration: 150

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        var new_idx = list.indexAt(mouse.x, list.contentY + mouse.y);
                        if (new_idx < 0)
                            return;

                        list.currentIndex = new_idx;
                        root.select(new_idx);
                    }
                    cursorShape: Qt.PointingHandCursor
                }
            }
        }

    }

    Component {
        id: listItem

        Rectangle {
            readonly property bool highlighted: ListView.isCurrentItem || mouseArea.containsMouse

            width: ListView.view.width
            height: root.itemHeight
            color: highlighted ? "#dedede" : "#eee"

            Text {
                id: label

                anchors.right: parent.right
                anchors.rightMargin: vpx(24)
                anchors.verticalCenter: parent.verticalCenter

                text: model.name
                color: "#444"
                font.pixelSize: root.textSize
                font.family: globalFonts.sans
            }

            MouseArea {
                id: mouseArea
                anchors.fill: parent
                hoverEnabled: true
            }
        }
    }


    states: State {
        name: "open"
        AnchorChanges {
            target: box
            anchors.left: undefined
            anchors.right: root.right
        }
    }

    readonly property var bezierDecelerate: [ 0,0, 0.2,1, 1,1 ]
    readonly property var bezierSharp: [ 0.4,0, 0.6,1, 1,1 ]

    transitions: [
        Transition {
            from: ""; to: "open"
            AnchorAnimation {
                duration: 175
                easing { type: Easing.Bezier; bezierCurve: bezierDecelerate }
            }
        },
        Transition {
            id: animClosing
            from: "open"; to: ""
            AnchorAnimation {
                duration: 150
                easing { type: Easing.Bezier; bezierCurve: bezierSharp }
            }
        }
    ]
}
