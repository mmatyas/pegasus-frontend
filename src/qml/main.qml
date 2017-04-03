import QtQuick 2.6
import QtQuick.Window 2.2


Window {
    visible: true
    width: 1280
    height: 720
    title: "Pegasus"
    color: "#222"


    PlatformBar {
        id: topbar
        z: 2
    }

    ListView {
        id: contentView
        width: parent.width
        anchors {
            top: topbar.bottom
            bottom: parent.bottom
        }

        model: pegasus.platforms
        delegate: contentViewDelegate

        orientation: ListView.Horizontal
        snapMode: ListView.SnapOneItem
        clip: true

        // onCurrentIndexChanged: contentView.positionViewAtIndex(currentIndex, ListView.Contain)
    }

    Component {
        id: contentViewDelegate

        Item {
            width: ListView.view.width
            height: ListView.view.height

            Item {
                width: parent.width * 0.35
                height: parent.height
                anchors {
                    left: parent.left
                }

                GamePreview {
                    anchors.fill: parent
                    model: gameModel
                }
            }
            Item {
                width: parent.width * 0.65
                height: parent.height
                anchors.right: parent.right

                GameGrid {
                    anchors.fill: parent
                }
            }
        }
    }
}
