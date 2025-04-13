// Pegasus Frontend
// Copyright (C) 2017-2019  Mátyás Mustoha
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


// FIXME: This file is mostly a duplicate of the other dialogs
import QtQuick 2.0


FocusScope {
    id: root

    property alias title: titleText.text
    property alias message: messageText.text

    property int textSize: vpx(18)
    property int titleTextSize: vpx(20)

    signal close()

    anchors.fill: parent
    visible: shade.opacity > 0

    focus: true
    onActiveFocusChanged: state = activeFocus ? "open" : ""

    Keys.onPressed: event => {
        if (api.keys.isCancel(event) && !event.isAutoRepeat) {
            event.accepted = true;
            root.close();
        }
    }



    Shade {
        id: shade
        onCancel: root.close()
    }

    // actual dialog
    MouseArea {
        anchors.centerIn: parent
        width: dialogBox.width
        height: dialogBox.height
    }
    Column {
        id: dialogBox

        width: parent.height * 0.8
        anchors.centerIn: parent
        scale: 0.5

        Behavior on scale { NumberAnimation { duration: 125 } }

        // title bar
        Rectangle {
            id: titleBar
            width: parent.width
            height: root.titleTextSize * 2.25
            color: "#333"

            Text {
                id: titleText

                anchors {
                    verticalCenter: parent.verticalCenter
                    left: parent.left
                    leftMargin: root.titleTextSize * 0.75
                }

                color: "#eee"
                font {
                    bold: true
                    pixelSize: root.titleTextSize
                    family: globalFonts.sans
                }
            }
        }


        // text area
        Rectangle {
            width: parent.width
            height: messageText.height + 3 * root.textSize
            color: "#555"

            Text {
                id: messageText

                anchors.centerIn: parent
                width: parent.width - 2 * root.textSize

                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter

                color: "#eee"
                font {
                    pixelSize: root.textSize
                    family: globalFonts.sans
                }
            }
        }

        // button row
        Rectangle {
            id: closeButton
            width: parent.width
            height: root.textSize * 2.25
            color: (focus || closeMouseArea.containsMouse) ? "#4ae" : "#666"

            focus: true

            Keys.onPressed: event => {
                if (api.keys.isAccept(event) && !event.isAutoRepeat) {
                    event.accepted = true;
                    root.close();
                }
            }

            Text {
                anchors.centerIn: parent

                text: qsTr("Ok") + api.tr
                color: "#eee"
                font {
                    pixelSize: root.textSize
                    family: globalFonts.sans
                }
            }

            MouseArea {
                id: closeMouseArea
                anchors.fill: parent
                hoverEnabled: true
                onClicked: root.close()
            }
        }
    }

    states: [
        State {
            name: "open"
            PropertyChanges { target: shade; opacity: 0.8 }
            PropertyChanges { target: dialogBox; scale: 1 }
        }
    ]
}
