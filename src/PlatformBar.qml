import QtQuick 2.0


Rectangle {
    width: parent.width
    height: 56
    color: "#333"

    PathView {
        model: 30
        delegate: platformCardDelegate

        path: Path {
            startX: -400
            startY: 28

            PathAttribute { name: "itemZ"; value: 201 }
            PathLine { x: parent.width * 0.2; y: 28 }

            PathPercent { value: 0.04 }
            PathAttribute { name: "itemZ"; value: 200 }
            PathLine { x: parent.width * 0.23; y: 12 }

            PathPercent { value: 0.08 }
            PathLine { x: parent.width * 0.7; y: 12 }

            PathPercent { value: 1 }
            PathAttribute { name: "itemZ"; value: 5 }
        }

        pathItemCount: 20

        snapMode: PathView.SnapOneItem
        preferredHighlightBegin: 0.04
        preferredHighlightEnd: 0.05

        focus: true
        Keys.onRightPressed: incrementCurrentIndex()
        Keys.onLeftPressed: decrementCurrentIndex()
    }

    Component {
        id: platformCardDelegate

        PlatformCard {
            text: modelData
            visible: PathView.onPath
            z: PathView.itemZ
        }
    }
}
