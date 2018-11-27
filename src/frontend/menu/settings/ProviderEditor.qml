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
        height: parent.height * 0.7
        width: height
        color: "#444"

        radius: vpx(8)

        anchors.centerIn: parent


        MouseArea {
            anchors.fill: parent
        }

        Text {
            id: info

            text: qsTr("When looking for games, Pegasus can use the following data sources, if enabled:") + api.tr
            color: "#eee"
            font.family: globalFonts.sans
            font.pixelSize: vpx(18)
            lineHeight: 1.15

            anchors.top: parent.top
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width * 0.85
            padding: font.pixelSize * lineHeight

            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            wrapMode: Text.WordWrap
        }

        Rectangle {
            anchors.top: info.bottom
            anchors.bottom: footer.top
            width: parent.width - vpx(30)
            anchors.horizontalCenter: parent.horizontalCenter
            color: "#333"

            Text {
                text: qsTr("(nothing on this platform)") + api.tr
                font.family: globalFonts.sans
                font.pixelSize: vpx(18)
                color: "#ccc"

                width: parent.width * 0.88
                anchors.centerIn: parent
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter

                visible: api.internal.settings.providers.count === 0
            }

            ListView {
                id: list
                anchors.fill: parent
                clip: true

                model: api.internal.settings.providers.model
                delegate: listEntry

                focus: true
                highlightRangeMode: ListView.ApplyRange
                preferredHighlightBegin: height * 0.5 - vpx(18) * 1.25
                preferredHighlightEnd: height * 0.5 + vpx(18) * 1.25
                highlightMoveDuration: 0
            }
        }

        Item {
            id: footer

            width: parent.width
            height: vpx(15)
            anchors.bottom: parent.bottom
        }
    }

    Component {
        id: listEntry

        FocusScope {
            readonly property int myIndex: index
            readonly property bool highlighted: ListView.view.focus
                                                && (ListView.isCurrentItem || mouseArea.containsMouse)

            width: parent.width
            height: label.height

            Rectangle {
                anchors.fill: parent
                color: "#555"
                visible: parent.highlighted
            }

            Text {
                id: label
                text: modelData.name
                verticalAlignment: Text.AlignVCenter
                lineHeight: 2.5

                color: "#eee"
                font.family: globalFonts.sans
                font.pixelSize: vpx(18)

                width: parent.width
                leftPadding: parent.height * 0.5
                rightPadding: leftPadding
                elide: Text.ElideRight
            }

            Switch {
                id: onoff
                anchors.right: parent.right
                anchors.rightMargin: label.rightPadding
                anchors.verticalCenter: parent.verticalCenter

                height: label.font.pixelSize * 1.25

                focus: true
                checked: modelData.enabled
                onCheckedChanged: modelData.enabled = checked
            }

            MouseArea {
                id: mouseArea
                anchors.fill: parent
                hoverEnabled: true

                onClicked: {
                    list.currentIndex = myIndex;
                    onoff.checked = !onoff.checked;
                }
            }
        }
    }

    NeedsRestartNotice {}
}
