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

    signal close

    anchors.fill: parent

    enabled: focus
    visible: opacity > 0.001
    opacity: focus ? 1.0 : 0.0
    Behavior on opacity { PropertyAnimation { duration: 150 } }

    Keys.onPressed: event => {
        if (api.keys.isCancel(event) && !event.isAutoRepeat) {
            event.accepted = true;
            root.close();
        }
    }


    SimpleShade {
        onClicked: root.close()
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

            text: qsTr("Some Android apps may not launch games unless you manually allow access "
                     + "to the game's directory, in both Pegasus and the launched app. Pegasus "
                     + "have permission for the following locations:") + api.tr
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

                    anchors.fill: parent
                    clip: true

                    model: Internal.settings.androidGrantedDirs
                    delegate: GameDirEditorEntry {}

                    focus: true
                    highlightRangeMode: ListView.ApplyRange
                    preferredHighlightBegin: height * 0.5 - vpx(18) * 1.25
                    preferredHighlightEnd: height * 0.5 + vpx(18) * 1.25
                    highlightMoveDuration: 0

                    KeyNavigation.right: buttonAdd

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            const new_idx = list.indexAt(mouse.x, list.contentY + mouse.y);
                            if (new_idx >= 0) {
                                list.currentIndex = new_idx;
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

                    icon: "+"
                    color: "#4c5"
                    onPressed: Internal.settings.requestAndroidDir()
                }
            }
        }
    }
}
