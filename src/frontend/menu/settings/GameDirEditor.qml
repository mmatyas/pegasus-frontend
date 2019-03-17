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


import "common"
import "gamedireditor"
import QtQuick 2.6


FocusScope {
    id: root

    signal close

    anchors.fill: parent

    enabled: focus
    visible: opacity > 0.001
    opacity: focus ? 1.0 : 0.0
    Behavior on opacity { PropertyAnimation { duration: 150 } }

    Keys.onPressed: {
        if (api.keys.isCancel(event) && !event.isAutoRepeat) {
            event.accepted = true;
            root.close();
        }
    }


    property var selectedIndices: [] // we don't have Set yet
    function isSelected(index) {
        return selectedIndices.indexOf(index) >= 0;
    }
    function toggleIndex(idx) {
        var arrayIdx = selectedIndices.indexOf(idx);
        if (arrayIdx > -1)
            selectedIndices.splice(arrayIdx, 1);
        else
            selectedIndices.push(idx);

        selectedIndicesChanged();
    }


    Timer {
        readonly property real step: interval / 1000

        id: deleteTimer
        interval: 16
        repeat: true
        onTriggered: deletionPercent += step
    }
    property real deletionPercent: 0.0

    function startDeletion() {
        deletionPercent = 0.0;
        deleteTimer.start();
    }
    function stopDeletion() {
        deleteTimer.stop();
        deletionPercent = 0.0;
    }
    onDeletionPercentChanged: {
        if (deletionPercent < 1.0)
            return;

        stopDeletion();
        api.internal.settings.removeGameDirs(selectedIndices);
        selectedIndices = [];
    }


    Rectangle {
        id: shade

        anchors.fill: parent
        color: "#000"
        opacity: 0.3

        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.LeftButton | Qt.RightButton
            onClicked: root.close()
        }
    }


    Rectangle {
        height: parent.height * 0.8
        width: height * 1.5
        color: "#444"

        radius: vpx(8)

        anchors.centerIn: parent

        // TODO: proper gamepad button mapping
        Keys.onPressed: {
            if (event.isAutoRepeat)
                return;

            var do_remove = event.key === Qt.Key_Delete || api.keys.isDetails(event);
            var do_add = api.keys.isFilters(event);
            if (!do_add && !do_remove)
                return;

            event.accepted = true;

            if (do_remove) {
                if (list.focus && !root.isSelected(list.currentIndex))
                    root.toggleIndex(list.currentIndex);

                root.startDeletion();
            }
            if (do_add)
                filePicker.focus = true;
        }
        Keys.onReleased: {
            if (event.isAutoRepeat)
                return;
            if (event.key !== Qt.Key_Delete && !api.keys.isDetails(event))
                return;

            event.accepted = true;
            root.stopDeletion();
        }


        MouseArea {
            anchors.fill: parent
        }

        Text {
            id: info

            text: qsTr("Pegasus will look for collection files (metadata.pegasus.txt or "
                     + "metadata.txt)\nin the following directories:") + api.tr
            color: "#eee"
            font.family: globalFonts.sans
            font.pixelSize: vpx(18)
            lineHeight: 1.15

            anchors.top: parent.top
            width: parent.width
            padding: font.pixelSize * lineHeight

            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            wrapMode: Text.WordWrap
        }

        Rectangle {
            anchors.top: info.bottom
            anchors.bottom: footer.top
            width: parent.width - vpx(40)
            anchors.horizontalCenter: parent.horizontalCenter
            color: "#333"

            ListView {
                id: list
                anchors.fill: parent
                clip: true

                model: api.internal.settings.gameDirs
                delegate: listEntry

                focus: true
                highlightRangeMode: ListView.ApplyRange
                preferredHighlightBegin: height * 0.5 - vpx(18) * 1.25
                preferredHighlightEnd: height * 0.5 + vpx(18) * 1.25
                highlightMoveDuration: 0

                KeyNavigation.down: buttonAdd

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        var new_idx = list.indexAt(mouse.x, list.contentY + mouse.y);
                        if (new_idx < 0)
                            return;

                        list.currentIndex = new_idx;
                        root.toggleIndex(new_idx);
                    }
                }
            }
        }

        Item {
            id: footer

            width: parent.width
            height: buttonRow.height * 1.75
            anchors.bottom: parent.bottom

            Row {
                id: buttonRow

                anchors.centerIn: parent
                spacing: height * 0.75

                GameDirEditorButton {
                    id: buttonAdd

                    image1: "qrc:/buttons/xb_y.png"
                    image2: "qrc:/buttons/ps_triangle.png"
                    text: qsTr("Add new") + api.tr

                    onPress: filePicker.focus = true

                    KeyNavigation.right: buttonDel
                }
                GameDirEditorButton {
                    id: buttonDel

                    image1: "qrc:/buttons/xb_x.png"
                    image2: "qrc:/buttons/ps_square.png"
                    text: qsTr("Remove selected") + api.tr

                    onPress: root.startDeletion();
                    onRelease: root.stopDeletion();

                    KeyNavigation.up: list
                }
            }
        }
    }

    Component {
        id: listEntry

        Rectangle {
            readonly property bool highlighted: ListView.view.focus
                                                && (ListView.isCurrentItem || mouseArea.containsMouse)
            readonly property bool selected: root.isSelected(index)

            width: parent.width
            height: label.height
            color: highlighted ? "#585858" : "transparent"

            Keys.onPressed: {
                if (api.keys.isAccept(event) && !event.isAutoRepeat) {
                    event.accepted = true;
                    root.toggleIndex(index);
                }
            }


            Rectangle {
                anchors.fill: parent
                color: "#d55"
                visible: parent.selected
            }
            Rectangle {
                id: deleteFill
                height: parent.height
                width: parent.width * deletionPercent
                color: "#924"
                visible: parent.selected && deleteTimer.running && width > 0
            }

            Text {
                id: label
                text: modelData
                verticalAlignment: Text.AlignVCenter
                lineHeight: 2

                color: "#eee"
                font.family: globalFonts.sans
                font.pixelSize: vpx(18)

                width: parent.width
                leftPadding: parent.height * 0.5
                rightPadding: leftPadding
                elide: Text.ElideRight
            }

            MouseArea {
                id: mouseArea
                anchors.fill: parent
                hoverEnabled: true
            }
        }
    }

    NeedsRestartNotice {}


    FilePicker {
        id: filePicker

        enabled: focus

        visible: opacity > 0.01
        opacity: focus ? 1.0 : 0.0
        Behavior on opacity { PropertyAnimation { duration: 300 } }

        onCancel: list.focus = true
        onPick: {
            api.internal.settings.addGameDir(dir_path);
            list.focus = true;
        }
    }
}
