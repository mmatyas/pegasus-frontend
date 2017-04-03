import QtQuick 2.0

ListView {
    delegate: Item {
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
