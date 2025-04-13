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


Rectangle {
    id: root

    signal activated

    readonly property bool highlighted: mouseArea.containsMouse || focus
    readonly property color colorOff: "#333"
    readonly property color colorOn: "#924"
    readonly property color colorHover: "#c55"
    property real activationPercent: 0.0

    Keys.onPressed: event => {
        if (api.keys.isAccept(event) && !event.isAutoRepeat) {
            event.accepted = true;
            root.startActivation();
        }
    }
    Keys.onReleased: {
        if (api.keys.isAccept(event) && !event.isAutoRepeat) {
            event.accepted = true;
            root.stopActivation();
        }
    }


    width: label.width + vpx(36)
    height: label.height
    radius: vpx(3)
    color: root.highlighted ? root.colorHover : root.colorOff
    border.width: root.highlighted ? 0 : vpx(1)
    border.color: "#666"

    Rectangle {
        width: root.activationPercent * parent.width
        height: parent.height
        radius: parent.radius
        color: root.colorOn
        visible: width > 0
    }

    Text {
        id: label
        text: qsTr("Reset") + api.tr
        color: "#eee"
        font.family: globalFonts.condensed
        font.pixelSize: vpx(18)
        lineHeight: 1.5
        verticalAlignment: Text.AlignVCenter
        anchors.centerIn: parent
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        onPressed: {
            root.forceActiveFocus();
            startActivation();
        }
        onReleased: stopActivation()
    }


    Timer {
        readonly property real step: interval / 1000

        id: activationTimer
        interval: 16
        repeat: true
        onTriggered: activationPercent += step
    }

    function startActivation() {
        activationPercent = 0.0;
        activationTimer.start();
    }
    function stopActivation() {
        activationTimer.stop();
        activationPercent = 0.0;
    }
    onActivationPercentChanged: {
        if (activationPercent < 1.0)
            return;

        stopActivation();
        root.activated();
    }
}
