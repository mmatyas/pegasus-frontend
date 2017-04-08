import QtQuick 2.6


ListView {
    model: pegasus.platforms
    delegate: gridListDelegate

    orientation: ListView.Horizontal
    snapMode: ListView.SnapOneItem
    clip: true

    interactive: false

    onCurrentIndexChanged: {
        appWindow.currentPlatform = currentItem.delegateModel;
        appWindow.currentGame = currentItem.selectedGame;
    }

    Component {
        id: gridListDelegate

        Item {
            width: ListView.view.width
            height: ListView.view.height

            visible: ListView.isCurrentItem

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
                        appWindow.currentGame = game
                }
            }
        }
    }
}
