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


import QtQuick 2.0

Item {
    id: root

    property int deviceId: -1
    property int deviceField: -1
    property bool fieldIsAxis: false

    readonly property int initialSeconds: 5
    property int remainingSeconds: initialSeconds

    signal close

    function triggerClose() {
        timeout.stop();
        root.close();
    }


    onActiveFocusChanged: if (activeFocus) {
        remainingSeconds = initialSeconds;
        timeout.restart();

        if (fieldIsAxis)
            Internal.gamepad.configureAxis(deviceId, deviceField);
        else
            Internal.gamepad.configureButton(deviceId, deviceField);
    }

    Keys.onPressed: event.accept = true
    Keys.onReleased: event.accept = true


    anchors.fill: parent

    enabled: focus
    visible: opacity > 0.001
    opacity: enabled ? 1.0 : 0.0
    Behavior on opacity { PropertyAnimation { duration: 150 } }


    Connections {
        target: Internal.gamepad
        function onButtonConfigured() { root.triggerClose(); }
        function onAxisConfigured() { root.triggerClose(); }
        function onConfigurationCanceled() { root.triggerClose(); }
    }


    Timer {
        id: timeout
        interval: 1000
        repeat: true
        onTriggered: {
            remainingSeconds--;
            if (remainingSeconds <= 0) {
                Internal.gamepad.cancelConfiguration();
                root.triggerClose();
                stop();
            }
        }
    }


    Rectangle {
        anchors.fill: parent
        color: "#000"
        opacity: 0.3

        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.AllButtons
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

            readonly property string baseText: root.fieldIsAxis
                ? QT_TR_NOOP("Tilt any axis now to record\n(timeout in %1 seconds)")
                : QT_TR_NOOP("Press any button now to record\n(timeout in %1 seconds)")

            text: qsTr(baseText).arg(root.remainingSeconds) + api.tr
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
