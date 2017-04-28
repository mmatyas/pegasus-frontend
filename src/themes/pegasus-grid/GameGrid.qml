import QtQuick 2.6


GridView {
    id: grid_root

    property var platformData: pegasus.currentPlatform
    // TODO: make these customizable
    property real columnCount: platformData
                               ? (platformData.shortName === "nes" ? 5 : 4)
                               : 1

    model: platformData ? platformData.games : 0
    onCurrentIndexChanged: pegasus.currentGameIndex = currentIndex;

    cellWidth: width / columnCount

    // Because the images are loaded asynchronously, we don't know the row heights,
    // and we don't want to hardcode per-platform settings either. As such, we have
    // to update cellHeight when an image gets loaded, by trying to fit the image
    // first by width, then adjusting cellHeight based on the images w/h ratio.

    // this is the max allowed height
    cellHeight: cellWidth * 2
    // on platform change, reset the height to avoid getting smaller and smaller
    onModelChanged: cellHeight = cellWidth * 2


    displayMarginBeginning: anchors.topMargin

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
            id: boxFront
            width: parent.GridView.isCurrentItem ? parent.width - rpx(8) : parent.width - rpx(16)
            height: parent.GridView.isCurrentItem ? parent.height - rpx(8) : parent.height - rpx(16)
            anchors.centerIn: parent

            asynchronous: true
            visible: model.assets.boxFront

            source: model.assets.boxFront ? "file:" + model.assets.boxFront : ""
            sourceSize { width: 256; height: 256 }
            fillMode: Image.PreserveAspectFit

            onStatusChanged: if (status === Image.Ready) {
                var img_ratio = paintedHeight / paintedWidth;
                var cell_ratio = grid_root.cellHeight / grid_root.cellWidth;
                if (img_ratio < cell_ratio)
                    grid_root.cellHeight = grid_root.cellWidth * img_ratio;
            }

            Behavior on width { PropertyAnimation { duration: 150 } }
            Behavior on height { PropertyAnimation { duration: 150 } }
        }

        Image {
            anchors.centerIn: parent

            visible: boxFront.status === Image.Loading
            source: "/common/loading-spinner.png"

            RotationAnimator on rotation {
                loops: Animator.Infinite;
                from: 0;
                to: 360;
                duration: 500
            }
        }

        Text {
            width: parent.width - rpx(64)
            anchors.centerIn: parent

            visible: !model.assets.boxFront

            text: model.title
            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignHCenter
            color: "#eee"
            font {
                pixelSize: rpx(16)
                family: "Roboto"
            }
        }

        MouseArea {
            anchors.fill: parent
            onClicked: parent.GridView.view.currentIndex = index
        }
    }
}
