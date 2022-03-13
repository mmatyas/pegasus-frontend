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
import QtQuick.Layouts 1.3


FocusScope {
    id: root

    property bool mSettingsChanged: false

    signal close

    anchors.fill: parent

    enabled: focus
    visible: opacity > 0.001
    opacity: focus ? 1.0 : 0.0
    Behavior on opacity { PropertyAnimation { duration: 150 } }

    function closeMaybe() {
        if (mSettingsChanged)
            reloadDialog.focus = true;
        else
            root.close();
    }

    Keys.onPressed: {
        if (api.keys.isCancel(event) && !event.isAutoRepeat) {
            event.accepted = true;
            root.closeMaybe();
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


    Rectangle {
        id: shade

        anchors.fill: parent
        color: "#000"
        opacity: 0.3

        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.LeftButton | Qt.RightButton
            onClicked: root.closeMaybe()
        }
    }


    Rectangle {
        id: main

        readonly property int borderSize: vpx(10)

        height: parent.height * 0.8
        width: Math.min(height * 1.5, parent.width)
        anchors.centerIn: parent

        color: "#444"
        radius: vpx(8)

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
            anchors.left: parent.left
            anchors.right: parent.right
            padding: font.pixelSize * lineHeight

            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            wrapMode: Text.WordWrap
        }

        RowLayout {
            anchors.top: info.bottom
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: main.borderSize
            anchors.topMargin: 0

            spacing: main.borderSize

            Rectangle {
                Layout.fillHeight: true
                Layout.fillWidth: true
                color: "#333"

                ListView {
                    id: list

                    property bool isComplete: false
                    Component.onCompleted: isComplete = true

                    anchors.fill: parent
                    clip: true

                    model: Internal.settings.gameDirs
                    delegate: GameDirEditorEntry {
                        selected: root.isSelected(index)
                        onPressed: root.toggleIndex(index)
                    }

                    focus: true
                    highlightRangeMode: ListView.ApplyRange
                    preferredHighlightBegin: height * 0.5 - vpx(18) * 1.25
                    preferredHighlightEnd: height * 0.5 + vpx(18) * 1.25
                    highlightMoveDuration: 0

                    KeyNavigation.right: buttonAdd

                    onModelChanged: {
                        if (isComplete)
                            root.mSettingsChanged = true;
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            const new_idx = list.indexAt(mouse.x, list.contentY + mouse.y);
                            if (new_idx >= 0) {
                                list.currentIndex = new_idx;
                                root.toggleIndex(new_idx);
                            }
                        }
                    }
                }
            }

            Column {
                id: buttonArea

                Layout.fillHeight: true
                spacing: main.borderSize

                GameDirEditorButton {
                    id: buttonAdd

                    icon: "\u2795"
                    color: "#4c5"
                    onPressed: filePicker.focus = true

                    KeyNavigation.down: buttonDel
                }

                GameDirEditorButton {
                    id: buttonDel

                    icon: "\u2796"
                    color: "#e43"
                    onPressed: {
                        Internal.settings.removeGameDirs(root.selectedIndices);
                        root.selectedIndices = [];
                    }

                    KeyNavigation.left: list
                }
            }
        }
    }


    ReloadQuestion {
        id: reloadDialog
        onAccept: {
            list.focus = true;
            root.mSettingsChanged = false;
            Internal.settings.reloadProviders();
        }
        onCancel: {
            list.focus = true;
            root.mSettingsChanged = false;
            root.close();
        }
    }


    FilePicker {
        id: filePicker

        enabled: focus

        visible: opacity > 0.01
        opacity: focus ? 1.0 : 0.0
        Behavior on opacity { PropertyAnimation { duration: 300 } }

        onCancel: list.focus = true
        onPick: {
            Internal.settings.addGameDir(dir_path);
            list.focus = true;
        }
    }
}
