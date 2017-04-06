import QtQuick 2.6


GridView {
    // TODO: make this customizable
    cellWidth: width * (shortName === "nes" ? 0.2 : 0.25)
    cellHeight: height * 0.3

    property variant selectedGame: null

    signal changed(int index, variant game)

    model: gameModel

    highlight: Rectangle {
        color: "#0074da"
        width: GridView.view.cellWidth
        height: GridView.view.cellHeight
    }

    onCurrentIndexChanged: {
        selectedGame = currentItem.delegateModel;
        changed(currentIndex, currentItem.delegateModel);
    }

    delegate: Item {
        id: gamegridDelegate
        width: GridView.view.cellWidth
        height: GridView.view.cellHeight

        property variant delegateModel: model

        Image {
            width: parent.GridView.isCurrentItem ? parent.width - 8 : parent.width - 16
            height: parent.GridView.isCurrentItem ? parent.height - 8 : parent.height - 16

            anchors.centerIn: parent

            asynchronous: true

            source: "file:" + model.assets.boxFront
            sourceSize { width: 256; height: 256 }

            fillMode: Image.PreserveAspectFit
        }

        MouseArea {
            anchors.fill: parent
            onClicked: parent.GridView.view.currentIndex = index
        }
    }
}
