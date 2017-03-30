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
                anchors.left: parent.left

                ListView {
                    y: 32
                    model: gameModel
                    delegate: gameInfoDelegate
                    anchors.fill: parent
                }

                Component {
                    id: gameInfoDelegate

                    Item {
                        width: ListView.view.width
                        height: ListView.view.height
                        x: 32

                        Image {
                            y: 64
                            source: "file:" + model.assets.logo
                            sourceSize.width: parent.width * 0.75

                            fillMode: Image.PreserveAspectFit
                        }

                        Text {
                            y: 220
                            color: "white"
                            text: model.title

                            font {
                                bold: true
                                pixelSize: 24
                                capitalization: Font.SmallCaps
                                family: "Roboto"
                            }
                        }
                        Text {
                            y: 256
                            color: "white"
                            text: model.description
                            width: parent.width * 0.75
                            wrapMode: Text.WordWrap

                            font {
                                pixelSize: 16
                                family: "Roboto"
                            }
                        }
                        Text {
                            y: 450
                            color: "white"
                            text: "Developer: " + model.developer
                            width: parent.width * 0.75
                            wrapMode: Text.WordWrap

                            font {
                                pixelSize: 16
                                family: "Roboto"
                            }
                        }
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

                Flow {
                    anchors {
                        fill: parent
                        topMargin: 32
                    }
                    spacing: 10

                    Repeater {
                        model: gameModel
                        delegate: gameGridDelegate

                    }
                }

                Component {
                    id: gameGridDelegate

                    Image {
                        source: "file:" + model.assets.boxFront
                        sourceSize.width: 200
                        sourceSize.height: 200

                        fillMode: Image.PreserveAspectFit
                    }
                }
            }
        }
    }
}
