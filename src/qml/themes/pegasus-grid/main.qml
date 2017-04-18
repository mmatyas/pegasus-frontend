import QtQuick 2.0

Item {
    focus: true
    Keys.forwardTo: [topbar, gamegrid]
    Keys.onReturnPressed: pegasus.launchGame();

    PlatformBar {
        id: topbar
        z: 300
        width: parent.width
    }

    BackgroundImage {
        anchors {
            top: topbar.bottom; bottom: parent.bottom
            left: parent.left; right: parent.right
        }
    }

    GameGrid {
        id: gamegrid
        z: 100
        width: parent.width * 0.65
        anchors {
            top: topbar.bottom; topMargin: 32
            right: parent.right
            bottom: parent.bottom
        }
    }

    GamePreview {
        z: 200
        width: (parent.width * 0.35) - anchors.leftMargin - 40
        anchors {
            top: topbar.bottom; topMargin: 32
            left: parent.left; leftMargin: 10
            bottom: parent.bottom
        }
    }
}
