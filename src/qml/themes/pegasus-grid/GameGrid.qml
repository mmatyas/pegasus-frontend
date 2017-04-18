import QtQuick 2.6


GridView {
    property var platformData: pegasus.currentPlatform
    // TODO: make these customizable
    property real rowCount: 3.4
    property real columnCount: platformData
                               ? (platformData.shortName === "nes" ? 5 : 4)
                               : 1


    cellWidth: width / columnCount
    cellHeight: height / rowCount
    displayMarginBeginning: anchors.topMargin

    model: platformData ? platformData.games : 0
    onCurrentIndexChanged: pegasus.currentGameIndex = currentIndex;

    highlight: Rectangle {
        color: "#0074da"
        width: cellWidth
        height: cellHeight
        scale: 1.05
    }

    delegate: Item {
        width: GridView.view.cellWidth
        height: GridView.view.cellHeight

        Image {
            width: parent.GridView.isCurrentItem ? parent.width - 8 : parent.width - 16
            height: parent.GridView.isCurrentItem ? parent.height - 8 : parent.height - 16
            anchors.centerIn: parent

            asynchronous: true
            visible: model.assets.boxFront

            source: model.assets.boxFront ? "file:" + model.assets.boxFront : ""
            sourceSize { width: 256; height: 256 }
            fillMode: Image.PreserveAspectFit

            Behavior on width { PropertyAnimation { duration: 150 } }
            Behavior on height { PropertyAnimation { duration: 150 } }
        }

        Text {
            width: parent.width - 64
            anchors.centerIn: parent

            visible: !model.assets.boxFront

            text: model.title
            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignHCenter
            color: "#eee"
            font {
                pixelSize: 16
                family: "Roboto"
            }
        }

        MouseArea {
            anchors.fill: parent
            onClicked: parent.GridView.view.currentIndex = index
        }
    }
}
