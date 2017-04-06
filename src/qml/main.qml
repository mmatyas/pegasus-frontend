import QtQuick 2.6
import QtQuick.Window 2.2


Window {
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

    ListView {
        id: gridList
        width: parent.width
        anchors {
            top: topbar.bottom
            bottom: parent.bottom
        }

        model: pegasus.platforms
        delegate: gridListDelegate

        orientation: ListView.Horizontal
        snapMode: ListView.SnapOneItem
        clip: true

        interactive: false

        onCurrentIndexChanged: {
            currentPlatform = currentItem.delegateModel;
            currentGame = currentItem.selectedGame;
        }
    }

    Component {
        id: gridListDelegate

        Item {
            width: ListView.view.width
            height: ListView.view.height

            property variant delegateModel: model
            property alias selectedGame: gamegrid.selectedGame

            GameGrid {
                id: gamegrid
                width: parent.width * 0.65
                height: parent.height
                anchors {
                    right: parent.right
                    top: parent.top; topMargin: 32
                    bottom: parent.bottom
                }
                displayMarginBeginning: anchors.topMargin

                onChanged: {
                    if (parent.ListView.isCurrentItem === true)
                        currentGame = game
                }
            }
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
