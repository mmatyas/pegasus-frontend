import QtQuick 2.0

Item {
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
