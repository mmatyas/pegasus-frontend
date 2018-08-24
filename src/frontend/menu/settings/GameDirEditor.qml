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

    signal close

    anchors.fill: parent


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


        MouseArea {
            anchors.fill: parent
        }


        Text {
            id: info

            text: "Pegasus will look for collection files (collections.pegasus.txt or "
                + "collections.txt)\nin the following directories:"
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

        Item {
            anchors.top: info.bottom
            anchors.bottom: footer.top
            width: parent.width - vpx(40)
            anchors.horizontalCenter: parent.horizontalCenter

            ListView {
                id: list
                anchors.fill: parent
                clip: true

                model: 10
                delegate: listEntry

                focus: true
                highlightRangeMode: ListView.ApplyRange
                preferredHighlightBegin: height * 0.5 - vpx(18) * 1.25
                preferredHighlightEnd: height * 0.5 + vpx(18) * 1.25
                highlightMoveDuration: 0

                KeyNavigation.down: buttonAdd
            }

            Rectangle {
                color: "transparent"
                border.width: vpx(1)
                border.color: "#aaa"

                anchors.fill: parent
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

                    image: "qrc:/buttons/ps_triangle.png"
                    text: qsTr("Add new") + api.tr

                    onPress: filePicker.focus = true

                    KeyNavigation.right: buttonDel
                }
                GameDirEditorButton {
                    id: buttonDel

                    image: "qrc:/buttons/ps_square.png"
                    text: qsTr("Remove selected") + api.tr

                    KeyNavigation.up: list
                }
            }
        }
    }

    Component {
        id: listEntry

        Rectangle {
            readonly property bool highlighted: ListView.isCurrentItem || mouseArea.containsMouse

            width: parent.width
            height: label.height
            color: highlighted ? "#585858" : "transparent"

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


    FilePicker {
        id: filePicker

        enabled: focus

        visible: opacity > 0.01
        opacity: focus ? 1.0 : 0.0
        Behavior on opacity { PropertyAnimation { duration: 300 } }

        onCancel: list.focus = true
        onPick: {
            console.log(url);
            list.focus = true;
        }
    }
}
