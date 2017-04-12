import QtQuick 2.6
import QtQuick.Window 2.2


Window {
    id: appWindow

    property var currentGame: null
    property var currentPlatform: null

    visible: true
    width: 1280
    height: 720
    title: "Pegasus"
    color: "#000"


    // wrapper to receive keyboard events
    Item {
        anchors.fill: parent

        focus: true
        Keys.forwardTo: [topbar, gamegrid]

        PlatformBar {
            id: topbar

            platformModel: pegasus.platforms
            onIndexChanged: {
                pegasus.currentPlatformIndex = index
                appWindow.currentPlatform = platform
            }

            z: 500
            width: parent.width
        }

        BackgroundImage {
            z: 100
            anchors {
                top: topbar.bottom; bottom: parent.bottom
                left: parent.left; right: parent.right
            }
        }

        GameGrid {
            id: gamegrid

            platformData: appWindow.currentPlatform
            onIndexChanged:  {
                pegasus.currentGameIndex = index
                appWindow.currentGame = game
            }

            z: 200
            width: parent.width * 0.65
            anchors {
                top: topbar.bottom; topMargin: 32
                right: parent.right
                bottom: parent.bottom
            }
        }

        GamePreview {
            gameData: appWindow.currentGame

            z: 400
            width: (parent.width * 0.35) - anchors.leftMargin - 40
            anchors {
                top: topbar.bottom; topMargin: 32
                left: parent.left; leftMargin: 10
                bottom: parent.bottom
            }
        }
    }
}
