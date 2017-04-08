import QtQuick 2.6
import QtQuick.Window 2.2


Window {
    id: appWindow
    visible: true
    width: 1280
    height: 720
    title: "Pegasus"
    color: "#181818"

    property variant currentGame: null
    property variant currentPlatform: null

    PlatformBar {
        id: topbar
        z: 2

        onChanged: {
            gridList.positionViewAtIndex(index, ListView.Contain);
            gridList.currentIndex = index;
        }
    }

    GameGridList {
        id: gridList
        anchors {
            top: topbar.bottom
            bottom: parent.bottom
            left: parent.left; right: parent.right
        }
    }

    GamePreview {
        id: gamepreview
        gameData: currentGame

        width: (parent.width * 0.35) - anchors.leftMargin - 48
        anchors {
            left: parent.left; leftMargin: 10
            top: topbar.bottom; topMargin: 32
            bottom: parent.bottom
        }
    }
}
