// Pegasus Frontend
// Copyright (C) 2017-2021  Mátyás Mustoha
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


import "common"
import "gamepad"
import "qrc:/qmlutils" as PegasusUtils
import Pegasus.Model 0.12
import QtQuick 2.8
import QtQuick.Window 2.2
import Qt.labs.qmlmodels 1.0


FocusScope {
    id: root

    readonly property var currentPad: gamepadView.model.get(gamepadView.currentIndex)

    signal close

    anchors.fill: parent
    enabled: focus
    visible: 0 < (x + width) && x < Window.window.width


    ListModel {
        id: fieldList

        ListElement {
            name: QT_TR_NOOP("Up")
            icon: "dpup"
            field: GamepadManager.GMButton.Up
        }
        ListElement {
            name: QT_TR_NOOP("Down")
            icon: "dpdown"
            field: GamepadManager.GMButton.Down
        }
        ListElement {
            name: QT_TR_NOOP("Left")
            icon: "dpleft"
            field: GamepadManager.GMButton.Left
        }
        ListElement {
            name: QT_TR_NOOP("Right")
            icon: "dpright"
            field: GamepadManager.GMButton.Right
        }

        ListElement {
            name: QT_TR_NOOP("A")
            icon: "a"
            field: GamepadManager.GMButton.South
        }
        ListElement {
            name: QT_TR_NOOP("B")
            icon: "b"
            field: GamepadManager.GMButton.East
        }
        ListElement {
            name: QT_TR_NOOP("X")
            icon: "x"
            field: GamepadManager.GMButton.West
        }
        ListElement {
            name: QT_TR_NOOP("Y")
            icon: "y"
            field: GamepadManager.GMButton.North
        }

        ListElement {
            name: QT_TR_NOOP("Start")
            icon: "start"
            field: GamepadManager.GMButton.Start
        }
        ListElement {
            name: QT_TR_NOOP("Guide")
            icon: "guide"
            field: GamepadManager.GMButton.Guide
        }
        ListElement {
            name: QT_TR_NOOP("Select")
            icon: "select"
            field: GamepadManager.GMButton.Select
        }

        ListElement {
            name: QT_TR_NOOP("Left Shoulder")
            icon: "lb"
            field: GamepadManager.GMButton.L1
        }
        ListElement {
            name: QT_TR_NOOP("Left Trigger")
            icon: "lt"
            field: GamepadManager.GMButton.L2
        }
        ListElement {
            name: QT_TR_NOOP("Right Shoulder")
            icon: "rb"
            field: GamepadManager.GMButton.R1
        }
        ListElement {
            name: QT_TR_NOOP("Right Trigger")
            icon: "rt"
            field: GamepadManager.GMButton.R2
        }

        ListElement {
            name: QT_TR_NOOP("Left X")
            icon: "lsx"
            isAxis: true
            field: GamepadManager.GMAxis.LeftX
        }
        ListElement {
            name: QT_TR_NOOP("Left Y")
            icon: "lsy"
            isAxis: true
            field: GamepadManager.GMAxis.LeftY
        }
        ListElement {
            name: QT_TR_NOOP("Left Press")
            icon: "lsb"
            field: GamepadManager.GMButton.L3
        }

        ListElement {
            name: QT_TR_NOOP("Right X")
            icon: "rsx"
            isAxis: true
            field: GamepadManager.GMAxis.RightX
        }
        ListElement {
            name: QT_TR_NOOP("Right Y")
            icon: "rsy"
            isAxis: true
            field: GamepadManager.GMAxis.RightY
        }
        ListElement {
            name: QT_TR_NOOP("Right Press")
            icon: "rsb"
            field: GamepadManager.GMButton.R3
        }
    }

    Item {
        id: contentContainer

        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        width: Math.min(parent.width, height * 16/9)

        z: 100

        ScreenHeader {
            id: screenTitle
            text: qsTr("Settings / Gamepad Layout") + api.tr
            z: 2
        }

        Item {
            id: leftContainer

            anchors.left: parent.left
            anchors.leftMargin: parent.width * 0.08
            anchors.right: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            height: childrenRect.height

            ListView {
                id: gamepadView

                anchors.left: parent.left
                anchors.right: parent.right
                height: vpx(80)
                clip: true

                highlightRangeMode: ListView.ApplyRange
                highlightMoveDuration: 300
                orientation: ListView.Horizontal

                focus: true
                KeyNavigation.down: fieldView

                model: api.internal.gamepad.devices
                delegate: Text {
                    text: modelData.name
                    color: "#eee"

                    width: ListView.view.width
                    height: ListView.view.height
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight

                    leftPadding: vpx(26)
                    rightPadding: leftPadding

                    font.family: global.fonts.condensed
                    font.pixelSize: vpx(32)
                }

                Rectangle {
                    anchors.fill: parent
                    color: "#404040"
                    z: -1
                    visible: parent.activeFocus
                }
            }

            ListView {
                id: fieldView

                readonly property int itemHeight: vpx(48)
                readonly property int visibleItems: 8

                function queryMapping(pad, field, isAxis) {
                    if (!pad)
                        return [null, null];

                    const map_str = isAxis
                        ? api.internal.gamepad.mappingForAxis(pad.deviceId, field)
                        : api.internal.gamepad.mappingForButton(pad.deviceId, field);

                    const map_kind = map_str.charAt(0);
                    const map_val = map_str.substring(1);
                    switch (map_kind) {
                        case 'h': return [QT_TR_NOOP("Hat %1"), map_val];
                        case 'b': return [QT_TR_NOOP("Button %1"), map_val];
                        case 'a': return [QT_TR_NOOP("Axis %1"), map_val];
                        default: return [null, null];
                    }
                }

                model: fieldList
                delegate: GamepadField {
                    label: qsTr(model.name) + api.tr
                    icon: model.icon
                    mapping: {
                        const [label, value] = fieldView.queryMapping(currentPad, model.field, model.isAxis);
                        return label
                            ? qsTr(label).arg(value) + api.tr
                            : "";
                    }
                }

                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: gamepadView.bottom
                height: itemHeight * visibleItems
                clip: true

                highlightRangeMode: ListView.ApplyRange
                highlightMoveDuration: 0
                preferredHighlightBegin: 2 * itemHeight
                preferredHighlightEnd: height - preferredHighlightBegin
            }
        }

        Item {
            id: rightContainer

            anchors.left: parent.horizontalCenter
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            height: parent.height * 0.75

            GamepadPreview {
                width: parent.width * 0.6
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter

                currentField: (fieldView.activeFocus && fieldView.currentIndex >= 0)
                    ? fieldView.model.get(fieldView.currentIndex).icon
                    : ""
            }
        }

        Item {
            id: footer

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: vpx(50)

            Rectangle {
                id: separator
                width: parent.width * 0.97
                height: vpx(1)
                color: "#777"
                anchors.top: parent.top
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Row {
                anchors.right: separator.right
                anchors.verticalCenter: parent.verticalCenter

                Item {
                    height: label.height
                    width: height

                    Image {
                        source: "qrc:/frontend/assets/gamepad/x360/b.png"

                        anchors.fill: parent
                        anchors.margins: parent.height * 0.1
                        anchors.verticalCenter: parent.verticalCenter
                        fillMode: Image.PreserveAspectFit
                    }
                }

                Text {
                    id: label

                    text: qsTr("Back") + api.tr
                    color: "#777"

                    font.family: global.fonts.condensed
                    font.pixelSize: vpx(22)
                    font.capitalization: Font.AllUppercase

                    lineHeight: 1.5
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }
    }


    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.RightButton
        onClicked: root.close()
    }

    PegasusUtils.HorizontalSwipeArea {
        anchors.fill: parent
        onSwipeRight: root.close()
    }
}
