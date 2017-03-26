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
    }

    Component {
        id: contentViewDelegate

        Item {
            width: ListView.view.width
            height: ListView.view.height

            Item {
                width: parent.width * 0.35
                height: parent.height
                anchors.left: parent.left

                ListView {
                    y: 50
                    model: gameModel
                    delegate: gameInfoDelegate
                    height: parent.height
                }

                Component {
                    id: gameInfoDelegate

                    Text {
                        color: "white"
                        text: model.title
                    }
                }
            }
            Item {
                width: parent.width * 0.65
                height: parent.height
                anchors.right: parent.right

                Text {
                    color: "white"
                    text: gameCount
                }

                ListView {
                    y: 50
                    model: gameModel
                    delegate: gameGridDelegate
                    height: parent.height
                }

                Component {
                    id: gameGridDelegate

                    Text {
                        color: "white"
                        text: model.title
                    }
                }
            }
        }
    }
}
