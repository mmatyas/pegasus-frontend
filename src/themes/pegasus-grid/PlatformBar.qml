import QtQuick 2.0


Rectangle {
    property var platformModel: pegasus.platforms

    color: "#333"
    height: rpx(54)

    Keys.forwardTo: platformPath

    PathView {
        id: platformPath
        model: platformModel
        delegate: platformCardDelegate

        path: Path {
            startX: rpx(-400)
            startY: rpx(36)

            PathAttribute { name: "itemZ"; value: 201 }
            PathLine { x: parent.width * 0.2; y: rpx(36) }

            PathPercent { value: 0.04 }
            PathAttribute { name: "itemZ"; value: 200 }
            PathLine { x: parent.width * 0.23; y: rpx(18) }

            PathPercent { value: 0.08 }
            PathLine { x: parent.width * 0.7; y: rpx(18) }

            PathPercent { value: 1 }
            PathAttribute { name: "itemZ"; value: rpx(5) }
        }

        pathItemCount: 20

        snapMode: PathView.SnapOneItem
        preferredHighlightBegin: 0.04
        preferredHighlightEnd: 0.05

        Keys.onPressed: {
            const qaed = [Qt.Key_Q, Qt.Key_A, Qt.Key_E, Qt.Key_D]; // QWERTx/AZERTY support
            if (qaed.indexOf(event.key) >= 0) {
                event.accepted = true;

                switch (event.key) {
                case Qt.Key_Q:
                case Qt.Key_A:
                    decrementCurrentIndex();
                    break;
                case Qt.Key_E:
                case Qt.Key_D:
                    incrementCurrentIndex();
                    break;
                }
            }
        }

        onCurrentIndexChanged: pegasus.currentPlatformIndex = currentIndex
    }

    Component {
        id: platformCardDelegate

        PlatformCard {
            platformShortName: shortName
            isOnTop: PathView.isCurrentItem

            visible: PathView.onPath

            z: PathView.itemZ
            width: parent.parent.width * 0.5
            height: rpx(72)
        }
    }
}
