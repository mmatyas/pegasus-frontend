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

FocusScope {
    id: root

    property int eventId
    property int keyToChange
    readonly property var modifiers: [Qt.Key_Control, Qt.Key_Alt, Qt.Key_AltGr, Qt.Key_Shift, Qt.Key_Meta]

    property bool recoding: false
    property int secondsLeft: 5

    signal close
    function triggerClose() {
        recoding = false;
        secondsLeft = 5;
        timeout.stop();
        root.close();
    }

    onFocusChanged: {
        if (focus) {
            secondsLeft = 5;
            timeout.restart();
        }
    }

    Keys.onPressed: {
        if (event.isAutoRepeat)
            return;

        if (modifiers.indexOf(event.key) > -1)
            return;

        if (keyToChange)
            api.internal.settings.keyEditor.replaceKeyCode(eventId, keyToChange, event);
        else
            api.internal.settings.keyEditor.addKey(eventId, event);

        triggerClose();
        event.accepted = true;
    }


    anchors.fill: parent

    enabled: focus
    visible: opacity > 0.001
    opacity: focus ? 1.0 : 0.0
    Behavior on opacity { PropertyAnimation { duration: 150 } }


    Timer {
        id: timeout
        interval: 1000
        repeat: true
        onTriggered: {
            secondsLeft -= 1;
            if (secondsLeft <= 0) {
                root.triggerClose();
                stop();
            }
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
            onClicked: root.triggerClose()
        }
    }

    Rectangle {
        anchors.centerIn: parent
        width: promptText.width + radius * 4
        height: promptText.height + radius * 2
        radius: vpx(10)
        color: "#444"

        Text {
            id: promptText
            text: qsTr("Press any key now to record\n(timeout in %1 seconds)").arg(root.secondsLeft) + api.tr
            color: "#eee"
            font.family: globalFonts.sans
            font.pixelSize: vpx(16)
            lineHeight: 1.25
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            anchors.centerIn: parent
        }
    }
}
