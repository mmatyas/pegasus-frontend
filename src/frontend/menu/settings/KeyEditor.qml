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


import "qrc:/qmlutils" as PegasusUtils
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
        if (api.keys.isCancel(event.key)) {
            event.accepted = true;
            root.close();
        }
    }


    PegasusUtils.HorizSwipeArea {
        anchors.fill: parent
        onSwipeRight: root.close()
    }

    Rectangle {
        anchors.fill: parent
        color: "#222"

        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.RightButton
            onClicked: root.close()
        }
    }

    FocusScope {
        id: content

        focus: true
        enabled: focus

        anchors.top: header.bottom
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        width: parent.width * 0.5

        ListView {
            anchors.fill: parent
            anchors.topMargin: vpx(30)
            focus: true

            model: api.settings.keyEditor.eventCount
            delegate: KeyEditorEntry {
                eventId: modelData
                onEditKey: {
                    recoder.eventId = eventId;
                    recoder.keyToChange = key;
                    recoder.focus = true;
                }
                onNewKey: {
                    recoder.eventId = eventId;
                    recoder.keyToChange = 0;
                    recoder.focus = true;
                }
                onDelKey: api.settings.keyEditor.delKey(eventId, key)
            }

            spacing: vpx(15)
            displayMarginBeginning: header.height + anchors.topMargin
            highlightMoveDuration: 150
            highlightRangeMode: ListView.ApplyRange
            preferredHighlightBegin: height * 0.25
            preferredHighlightEnd: height * 0.75
        }
    }

    Rectangle {
        anchors.top: parent.top
        anchors.bottom: header.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        color: "#222"
        opacity: 0.75
    }

    ScreenHeader {
        id: header
        text: qsTr("Settings > Keyboard") + api.tr
    }

    KeyEditorRecorder {
        id: recoder
        onClose: content.focus = true
    }
}
