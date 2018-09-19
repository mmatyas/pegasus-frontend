import QtQuick 2.0


Item {
    property alias text: label.text
    property alias color: bgRect.color
    signal pressed

    readonly property bool highlighted: mouseArea.containsMouse || activeFocus

    Keys.onPressed: {
        if (api.keys.isAccept(event.key)) {
            event.accepted = true;
            pressed();
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: parent.pressed()
    }
    Rectangle {
        id: bgRect
        anchors.fill: parent
        opacity: parent.highlighted ? 0.75 : 0.25
    }
    Text {
        id: label
        color: "#eee"
        font.family: globalFonts.sans
        font.pixelSize: root.textSize
        anchors.centerIn: parent
    }
}
