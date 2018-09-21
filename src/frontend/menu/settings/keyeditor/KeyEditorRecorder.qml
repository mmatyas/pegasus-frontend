import QtQuick 2.0

FocusScope {
    id: root

    property int eventId
    property int keyToChange

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

        if (keyToChange)
            api.settings.keyEditor.changeKey(eventId, keyToChange, event.key);
        else
            api.settings.keyEditor.addKey(eventId, event.key);

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
