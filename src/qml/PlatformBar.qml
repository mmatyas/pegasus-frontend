import QtQuick 2.0


Rectangle {
    width: parent.width
    height: 54
    color: "#333"

    PathView {
        model: pegasus.platforms
        delegate: platformCardDelegate

        path: Path {
            startX: -400
            startY: 36

            PathAttribute { name: "itemZ"; value: 201 }
            PathLine { x: parent.width * 0.2; y: 36 }

            PathPercent { value: 0.04 }
            PathAttribute { name: "itemZ"; value: 200 }
            PathLine { x: parent.width * 0.23; y: 18 }

            PathPercent { value: 0.08 }
            PathLine { x: parent.width * 0.7; y: 18 }

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

        onCurrentIndexChanged: contentView.positionViewAtIndex(currentIndex, ListView.Contain)
    }

    Component {
        id: platformCardDelegate

        PlatformCard {
            platformShortName: shortName
            visible: PathView.onPath
            z: PathView.itemZ

            width: parent.parent.width * 0.5
            height: 72
        }
    }
}
