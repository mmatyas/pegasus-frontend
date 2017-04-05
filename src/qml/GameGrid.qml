import QtQuick 2.6


GridView {
    // TODO: make this customizable
    cellWidth: parent.width * (shortName === "nes" ? 0.2 : 0.25)
    cellHeight: parent.height * 0.3

    model: gameModel

    highlight: Rectangle {
        color: "#0074da"
        width: GridView.view.cellWidth
        height: GridView.view.cellHeight
    }

    delegate: Item {
        width: GridView.view.cellWidth
        height: GridView.view.cellHeight

        Image {
            width: parent.GridView.isCurrentItem ? parent.width - 10 : parent.width - 16
            height: parent.GridView.isCurrentItem ? parent.height - 10 : parent.height - 16

            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter

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
