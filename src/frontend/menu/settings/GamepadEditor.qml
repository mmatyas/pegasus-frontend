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


import "gamepad/preview" as GamepadPreview
import "gamepad"
import "qrc:/qmlutils" as PegasusUtils
import Pegasus.Model 0.12
import QtQuick 2.8


FocusScope {
    id: root

    signal close

    anchors.fill: parent
    enabled: focus
    visible: 0 < (x + width) && x < globalWidth

    function triggerClose() {
        root.stopEscapeTimer();
        root.close();
    }

    readonly property var gamepad: gamepadList.model.get(gamepadList.currentIndex)
    readonly property bool hasGamepads: gamepad !== null


    property ConfigField recordingField: null
    function recordConfig(configField) {
        // turn off the previously recording field
        if (recordingField !== null && configField !== recordingField)
            recordingField.recording = false;

        // turn on the currently recording one
        recordingField = configField
        if (recordingField !== null)
            recordingField.recording = true;
    }

    property real escapeDelay: 1500
    property real escapeStartTime: 0
    property real escapeProgress: 0

    Timer {
        id: escapeTimer
        interval: 50
        repeat: true
        onTriggered: {
            var currentTime = new Date().getTime();
            escapeProgress = (currentTime - escapeStartTime) / escapeDelay;

            if (escapeProgress > 1.0)
                root.triggerClose();
        }
    }

    function stopEscapeTimer() {
        escapeTimer.stop();
        escapeStartTime = 0;
        escapeProgress = 0;
    }

    Keys.onPressed: {
        if (api.keys.isCancel(event) && !event.isAutoRepeat) {
            event.accepted = true;
            escapeStartTime = new Date().getTime();
            escapeTimer.start();
        }
    }
    Keys.onReleased: {
        if (api.keys.isCancel(event) && !event.isAutoRepeat) {
            event.accepted = true;
            stopEscapeTimer();
        }
    }

    Connections {
        target: api.internal.gamepad
        onButtonConfigured: recordConfig(null)
        onAxisConfigured: recordConfig(null)
        onConfigurationCanceled: recordConfig(null)
    }

    Rectangle {
        id: deviceSelect
        width: parent.width
        height: vpx(70)
        color: "#333"
        anchors.top: parent.top

        focus: true
        Keys.forwardTo: [gamepadList]
        KeyNavigation.down: configL1

        GamepadName {
            visible: !hasGamepads
            highlighted: deviceSelect.focus
            text: qsTr("No gamepads connected") + api.tr
        }

        ListView {
            id: gamepadList
            anchors.fill: parent

            clip: true
            highlightRangeMode: ListView.StrictlyEnforceRange
            highlightMoveDuration: 300
            orientation: ListView.Horizontal

            model: api.internal.gamepad.devices

            delegate: Item {
                width: ListView.view.width
                height: ListView.view.height

                GamepadName {
                    text: "#" + (index + 1) + ": " + modelData.name
                    highlighted: deviceSelect.focus
                }
            }
        }
    }

    Rectangle {
        width: parent.width
        color: "#222"
        anchors {
            top: deviceSelect.bottom
            bottom: parent.bottom
        }
    }

    FocusScope {
        id: layoutArea
        width: parent.width
        anchors {
            top: deviceSelect.bottom
            bottom: footer.top
        }

        property int horizontalOffset: vpx(-560)
        property int verticalSpacing: vpx(170)

        onActiveFocusChanged:
            if (!activeFocus) padPreview.currentButton = ""

        ConfigGroup {
            label: qsTr("left back") + api.tr
            anchors {
                left: parent.horizontalCenter
                leftMargin: parent.horizontalOffset
                verticalCenter: parent.verticalCenter
                verticalCenterOffset: -parent.verticalSpacing
            }

            ConfigField {
                focus: true
                id: configL1
                text: qsTr("shoulder") + api.tr
                onActiveFocusChanged:
                    if (activeFocus) padPreview.currentButton = "l1"

                pressed: gamepad && gamepad.buttonL1
                Keys.onPressed: if (api.keys.isAccept(event) && !event.isAutoRepeat) {
                    event.accepted = true;
                    recordConfig(this);
                }
                Keys.onReleased: if (api.keys.isAccept(event) && !event.isAutoRepeat) {
                    event.accepted = true;
                    api.internal.gamepad.configureButton(gamepad.deviceId, GamepadManager.GMButton.L1);
                }

                KeyNavigation.right: configSelect
                KeyNavigation.down: configL2
            }
            ConfigField {
                id: configL2
                text: qsTr("trigger") + api.tr
                onActiveFocusChanged:
                    if (activeFocus) padPreview.currentButton = "l2"

                pressed: gamepad && gamepad.buttonL2
                Keys.onPressed: if (api.keys.isAccept(event) && !event.isAutoRepeat) {
                    event.accepted = true;
                    recordConfig(this);
                }
                Keys.onReleased: if (api.keys.isAccept(event) && !event.isAutoRepeat) {
                    event.accepted = true;
                    api.internal.gamepad.configureButton(gamepad.deviceId, GamepadManager.GMButton.L2);
                }

                KeyNavigation.right: configR2
                KeyNavigation.down: configDpadUp
            }
        }

        ConfigGroup {
            label: qsTr("dpad") + api.tr
            anchors {
                left: parent.horizontalCenter
                leftMargin: parent.horizontalOffset
                verticalCenter: parent.verticalCenter
            }

            ConfigField {
                id: configDpadUp
                text: qsTr("up") + api.tr
                onActiveFocusChanged:
                    if (activeFocus) padPreview.currentButton = "dpup"

                pressed: gamepad && gamepad.buttonUp
                Keys.onPressed: if (api.keys.isAccept(event) && !event.isAutoRepeat) {
                    event.accepted = true;
                    recordConfig(this);
                }
                Keys.onReleased: if (api.keys.isAccept(event) && !event.isAutoRepeat) {
                    event.accepted = true;
                    api.internal.gamepad.configureButton(gamepad.deviceId, GamepadManager.GMButton.Up);
                }

                KeyNavigation.right: configA
                KeyNavigation.down: configDpadDown
            }
            ConfigField {
                id: configDpadDown
                text: qsTr("down") + api.tr
                onActiveFocusChanged:
                    if (activeFocus) padPreview.currentButton = "dpdown"

                pressed: gamepad && gamepad.buttonDown
                Keys.onPressed: if (api.keys.isAccept(event) && !event.isAutoRepeat) {
                    event.accepted = true;
                    recordConfig(this);
                }
                Keys.onReleased: if (api.keys.isAccept(event) && !event.isAutoRepeat) {
                    event.accepted = true;
                    api.internal.gamepad.configureButton(gamepad.deviceId, GamepadManager.GMButton.Down);
                }

                KeyNavigation.right: configB
                KeyNavigation.down: configDpadLeft
            }
            ConfigField {
                id: configDpadLeft
                text: qsTr("left") + api.tr
                onActiveFocusChanged:
                    if (activeFocus) padPreview.currentButton = "dpleft"

                pressed: gamepad && gamepad.buttonLeft
                Keys.onPressed: if (api.keys.isAccept(event) && !event.isAutoRepeat) {
                    event.accepted = true;
                    recordConfig(this);
                }
                Keys.onReleased: if (api.keys.isAccept(event) && !event.isAutoRepeat) {
                    event.accepted = true;
                    api.internal.gamepad.configureButton(gamepad.deviceId, GamepadManager.GMButton.Left);
                }

                KeyNavigation.right: configX
                KeyNavigation.down: configDpadRight
            }
            ConfigField {
                id: configDpadRight
                text: qsTr("right") + api.tr
                onActiveFocusChanged:
                    if (activeFocus) padPreview.currentButton = "dpright"

                pressed: gamepad && gamepad.buttonRight
                Keys.onPressed: if (api.keys.isAccept(event) && !event.isAutoRepeat) {
                    event.accepted = true;
                    recordConfig(this);
                }
                Keys.onReleased: if (api.keys.isAccept(event) && !event.isAutoRepeat) {
                    event.accepted = true;
                    api.internal.gamepad.configureButton(gamepad.deviceId, GamepadManager.GMButton.Right);
                }

                KeyNavigation.right: configY
                KeyNavigation.down: configLeftStickX
            }
        }

        ConfigGroup {
            label: qsTr("left stick") + api.tr
            anchors {
                left: parent.horizontalCenter
                leftMargin: parent.horizontalOffset
                verticalCenter: parent.verticalCenter
                verticalCenterOffset: parent.verticalSpacing
            }

            ConfigField {
                id: configLeftStickX
                text: qsTr("x axis") + api.tr
                onActiveFocusChanged:
                    if (activeFocus) padPreview.currentButton = "lx"

                pressed: gamepad && Math.abs(gamepad.axisLeftX) > 0.05
                Keys.onPressed: if (api.keys.isAccept(event) && !event.isAutoRepeat) {
                    event.accepted = true;
                    recordConfig(this);
                }
                Keys.onReleased: if (api.keys.isAccept(event) && !event.isAutoRepeat) {
                    event.accepted = true;
                    api.internal.gamepad.configureAxis(gamepad.deviceId, GamepadManager.GMAxis.LeftX);
                }

                KeyNavigation.right: configRightStickX
                KeyNavigation.down: configLeftStickY
            }
            ConfigField {
                id: configLeftStickY
                text: qsTr("y axis") + api.tr
                onActiveFocusChanged:
                    if (activeFocus) padPreview.currentButton = "ly"

                pressed: gamepad && Math.abs(gamepad.axisLeftY) > 0.05
                Keys.onPressed: if (api.keys.isAccept(event) && !event.isAutoRepeat) {
                    event.accepted = true;
                    recordConfig(this);
                }
                Keys.onReleased: if (api.keys.isAccept(event) && !event.isAutoRepeat) {
                    event.accepted = true;
                    api.internal.gamepad.configureAxis(gamepad.deviceId, GamepadManager.GMAxis.LeftY);
                }

                KeyNavigation.right: configRightStickY
                KeyNavigation.down: configL3
            }
            ConfigField {
                id: configL3
                text: qsTr("press") + api.tr
                onActiveFocusChanged:
                    if (activeFocus) padPreview.currentButton = "l3"

                pressed: gamepad && gamepad.buttonL3
                Keys.onPressed: if (api.keys.isAccept(event) && !event.isAutoRepeat) {
                    event.accepted = true;
                    recordConfig(this);
                }
                Keys.onReleased: if (api.keys.isAccept(event) && !event.isAutoRepeat) {
                    event.accepted = true;
                    api.internal.gamepad.configureButton(gamepad.deviceId, GamepadManager.GMButton.L3);
                }

                KeyNavigation.right: configR3
            }
        }

        ConfigGroup {
            label: qsTr("right back") + api.tr
            alignment: Text.AlignRight
            anchors {
                right: parent.horizontalCenter
                rightMargin: parent.horizontalOffset
                verticalCenter: parent.verticalCenter
                verticalCenterOffset: -parent.verticalSpacing
            }

            ConfigField {
                id: configR1
                text: qsTr("shoulder") + api.tr
                onActiveFocusChanged:
                    if (activeFocus) padPreview.currentButton = "r1"

                pressed: gamepad && gamepad.buttonR1
                Keys.onPressed: if (api.keys.isAccept(event) && !event.isAutoRepeat) {
                    event.accepted = true;
                    recordConfig(this);
                }
                Keys.onReleased: if (api.keys.isAccept(event) && !event.isAutoRepeat) {
                    event.accepted = true;
                    api.internal.gamepad.configureButton(gamepad.deviceId, GamepadManager.GMButton.R1);
                }

                KeyNavigation.down: configR2
            }
            ConfigField {
                id: configR2
                text: qsTr("trigger") + api.tr
                onActiveFocusChanged:
                    if (activeFocus) padPreview.currentButton = "r2"

                pressed: gamepad && gamepad.buttonR2
                Keys.onPressed: if (api.keys.isAccept(event) && !event.isAutoRepeat) {
                    event.accepted = true;
                    recordConfig(this);
                }
                Keys.onReleased: if (api.keys.isAccept(event) && !event.isAutoRepeat) {
                    event.accepted = true;
                    api.internal.gamepad.configureButton(gamepad.deviceId, GamepadManager.GMButton.R2);
                }

                KeyNavigation.down: configA
            }
        }

        ConfigGroup {
            label: qsTr("abxy") + api.tr
            alignment: Text.AlignRight
            anchors {
                right: parent.horizontalCenter
                rightMargin: parent.horizontalOffset
                verticalCenter: parent.verticalCenter
            }

            ConfigField {
                id: configA
                text: "a"
                onActiveFocusChanged:
                    if (activeFocus) padPreview.currentButton = "a"

                pressed: gamepad && gamepad.buttonSouth
                Keys.onPressed: if (api.keys.isAccept(event) && !event.isAutoRepeat) {
                    event.accepted = true;
                    recordConfig(this);
                }
                Keys.onReleased: if (api.keys.isAccept(event) && !event.isAutoRepeat) {
                    event.accepted = true;
                    api.internal.gamepad.configureButton(gamepad.deviceId, GamepadManager.GMButton.South);
                }

                KeyNavigation.down: configB
            }
            ConfigField {
                id: configB
                text: "b"
                onActiveFocusChanged:
                    if (activeFocus) padPreview.currentButton = "b"

                pressed: gamepad && gamepad.buttonEast
                Keys.onPressed: if (api.keys.isAccept(event) && !event.isAutoRepeat) {
                    event.accepted = true;
                    recordConfig(this);
                }
                Keys.onReleased: if (api.keys.isAccept(event) && !event.isAutoRepeat) {
                    event.accepted = true;
                    api.internal.gamepad.configureButton(gamepad.deviceId, GamepadManager.GMButton.East);
                }

                KeyNavigation.down: configX
            }
            ConfigField {
                id: configX
                text: "x"
                onActiveFocusChanged:
                    if (activeFocus) padPreview.currentButton = "x"

                pressed: gamepad && gamepad.buttonWest
                Keys.onPressed: if (api.keys.isAccept(event) && !event.isAutoRepeat) {
                    event.accepted = true;
                    recordConfig(this);
                }
                Keys.onReleased: if (api.keys.isAccept(event) && !event.isAutoRepeat) {
                    event.accepted = true;
                    api.internal.gamepad.configureButton(gamepad.deviceId, GamepadManager.GMButton.West);
                }

                KeyNavigation.down: configY
            }
            ConfigField {
                id: configY
                text: "y"
                onActiveFocusChanged:
                    if (activeFocus) padPreview.currentButton = "y"

                pressed: gamepad && gamepad.buttonNorth
                Keys.onPressed: if (api.keys.isAccept(event) && !event.isAutoRepeat) {
                    event.accepted = true;
                    recordConfig(this);
                }
                Keys.onReleased: if (api.keys.isAccept(event) && !event.isAutoRepeat) {
                    event.accepted = true;
                    api.internal.gamepad.configureButton(gamepad.deviceId, GamepadManager.GMButton.North);
                }

                KeyNavigation.down: configRightStickX
            }
        }

        ConfigGroup {
            label: qsTr("right stick") + api.tr
            alignment: Text.AlignRight
            anchors {
                right: parent.horizontalCenter
                rightMargin: parent.horizontalOffset
                verticalCenter: parent.verticalCenter
                verticalCenterOffset: parent.verticalSpacing
            }

            ConfigField {
                id: configRightStickX
                text: qsTr("x axis") + api.tr
                onActiveFocusChanged:
                    if (activeFocus) padPreview.currentButton = "rx"

                pressed: gamepad && Math.abs(gamepad.axisRightX) > 0.05
                Keys.onPressed: if (api.keys.isAccept(event) && !event.isAutoRepeat) {
                    event.accepted = true;
                    recordConfig(this);
                }
                Keys.onReleased: if (api.keys.isAccept(event) && !event.isAutoRepeat) {
                    event.accepted = true;
                    api.internal.gamepad.configureAxis(gamepad.deviceId, GamepadManager.GMAxis.RightX);
                }

                KeyNavigation.down: configRightStickY
            }
            ConfigField {
                id: configRightStickY
                text: qsTr("y axis") + api.tr
                onActiveFocusChanged:
                    if (activeFocus) padPreview.currentButton = "ry"

                pressed: gamepad && Math.abs(gamepad.axisRightY) > 0.05
                Keys.onPressed: if (api.keys.isAccept(event) && !event.isAutoRepeat) {
                    event.accepted = true;
                    recordConfig(this);
                }
                Keys.onReleased: if (api.keys.isAccept(event) && !event.isAutoRepeat) {
                    event.accepted = true;
                    api.internal.gamepad.configureAxis(gamepad.deviceId, GamepadManager.GMAxis.RightY);
                }

                KeyNavigation.down: configR3
            }
            ConfigField {
                id: configR3
                text: qsTr("press") + api.tr
                onActiveFocusChanged:
                    if (activeFocus) padPreview.currentButton = "r3"

                pressed: gamepad && gamepad.buttonR3
                Keys.onPressed: if (api.keys.isAccept(event) && !event.isAutoRepeat) {
                    event.accepted = true;
                    recordConfig(this);
                }
                Keys.onReleased: if (api.keys.isAccept(event) && !event.isAutoRepeat) {
                    event.accepted = true;
                    api.internal.gamepad.configureButton(gamepad.deviceId, GamepadManager.GMButton.R3);
                }
            }
        }

        Column {
            spacing: vpx(1)
            anchors {
                horizontalCenter: parent.horizontalCenter
                verticalCenter: parent.verticalCenter
                verticalCenterOffset: vpx(-220)
            }

            ConfigGroupLabel {
                text: qsTr("center") + api.tr
                anchors.horizontalCenter: parent.horizontalCenter
            }
            Row {
                spacing: vpx(1)
                property int alignment: Text.AlignHCenter

                ConfigField {
                    id: configSelect
                    text: qsTr("select") + api.tr
                    onActiveFocusChanged:
                        if (activeFocus) padPreview.currentButton = "select"

                    pressed: gamepad && gamepad.buttonSelect
                    Keys.onPressed: if (api.keys.isAccept(event) && !event.isAutoRepeat) {
                        event.accepted = true;
                        recordConfig(this);
                    }
                    Keys.onReleased: if (api.keys.isAccept(event) && !event.isAutoRepeat) {
                        event.accepted = true;
                        api.internal.gamepad.configureButton(gamepad.deviceId, GamepadManager.GMButton.Select);
                    }

                    KeyNavigation.up: deviceSelect
                    KeyNavigation.down: configL1
                    KeyNavigation.right: configGuide
                }
                ConfigField {
                    id: configGuide
                    text: qsTr("guide") + api.tr
                    onActiveFocusChanged:
                        if (activeFocus) padPreview.currentButton = "guide"

                    pressed: gamepad && gamepad.buttonGuide
                    Keys.onPressed: if (api.keys.isAccept(event) && !event.isAutoRepeat) {
                        event.accepted = true;
                        recordConfig(this);
                    }
                    Keys.onReleased: if (api.keys.isAccept(event) && !event.isAutoRepeat) {
                        event.accepted = true;
                        api.internal.gamepad.configureButton(gamepad.deviceId, GamepadManager.GMButton.Guide);
                    }

                    KeyNavigation.up: deviceSelect
                    KeyNavigation.right: configStart
                }
                ConfigField {
                    id: configStart
                    text: qsTr("start") + api.tr
                    onActiveFocusChanged:
                        if (activeFocus) padPreview.currentButton = "start"

                    pressed: gamepad && gamepad.buttonStart
                    Keys.onPressed: if (api.keys.isAccept(event) && !event.isAutoRepeat) {
                        event.accepted = true;
                        recordConfig(this);
                    }
                    Keys.onReleased: if (api.keys.isAccept(event) && !event.isAutoRepeat) {
                        event.accepted = true;
                        api.internal.gamepad.configureButton(gamepad.deviceId, GamepadManager.GMButton.Start);
                    }

                    KeyNavigation.up: deviceSelect
                    KeyNavigation.down: configR1
                    KeyNavigation.right: configR1
                }
            }


        }

        GamepadPreview.Container {
            id: padPreview
            gamepad: root.gamepad
        }
    }

    Item {
        id: footer
        width: parent.width
        height: vpx(50)
        anchors.bottom: parent.bottom

        Rectangle {
            width: parent.width * 0.97
            height: vpx(1)
            color: "#777"
            anchors.top: parent.top
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Canvas {
            width: backButtonIcon.width + vpx(4)
            height: width
            anchors.centerIn: backButtonIcon

            property real progress: escapeProgress
            onProgressChanged: requestPaint()

            onPaint: {
                var ctx = getContext('2d');
                ctx.clearRect(0, 0, width, height);

                var center = width / 2;
                var startAngle = -Math.PI / 2

                ctx.beginPath();
                ctx.fillStyle = "#eee";
                ctx.moveTo(center, center);
                ctx.arc(center, center, center,
                        startAngle, startAngle + Math.PI * 2 * progress, false);
                ctx.fill();
            }
        }

        // TODO: replace this with an SVG icon
        Rectangle {
            id: backButtonIcon
            height: label.height
            width: height
            radius: width * 0.5
            border { color: "#777"; width: vpx(1) }
            color: "transparent"

            anchors {
                right: label.left
                verticalCenter: parent.verticalCenter
                verticalCenterOffset: vpx(1)
                margins: vpx(10)
            }

            Text {
                text: "B"
                color: escapeStartTime ? "#eee" : "#777"
                font {
                    family: globalFonts.sans
                    pixelSize: parent.height * 0.7
                }
                anchors.centerIn: parent
            }
        }

        Text {
            id: label
            text: qsTr("hold down to quit") + api.tr
            verticalAlignment: Text.AlignTop

            color: escapeStartTime ? "#eee" : "#777"
            font {
                family: globalFonts.sans
                pixelSize: vpx(22)
                capitalization: Font.SmallCaps
            }
            anchors {
                verticalCenter: parent.verticalCenter
                verticalCenterOffset: vpx(-1)
                right: parent.right; rightMargin: parent.width * 0.015
            }
        }
    }

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.RightButton
        onClicked: root.triggerClose()
    }

    PegasusUtils.HorizontalSwipeArea {
        anchors.fill: parent
        onSwipeRight: root.triggerClose()
    }
}
