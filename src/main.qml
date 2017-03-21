import QtQuick 2.6
import QtQuick.Window 2.2

Window {
    visible: true
    width: 1280
    height: 720
    title: qsTr("Pegasus")
    color: "#222"

    PlatformBar {
        id: topbar
        z: 2
    }

    Item {
        width: parent.width
        anchors {
            top: topbar.bottom
            bottom: parent.bottom
        }
        Item {
            width: parent.width * 0.35
            height: parent.height
            anchors.left: parent.left
        }
        Item {
            width: parent.width * 0.65
            height: parent.height
            anchors.right: parent.right
        }
    }
}
