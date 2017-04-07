import QtQuick 2.0

Item {
    property variant gameData

    Column {
        width: parent.width
        height: parent.height
        spacing: 16

        visible: gameData

        Image {
            width: parent.width
            height: 192

            source: gameData ? (gameData.assets.logo ? "file:" + gameData.assets.logo : "") : ""
            sourceSize { width: 512; height: 192 }

            fillMode: Image.PreserveAspectFit
        }

        Text {
            width: parent.width

            color: "#eee"
            text: gameData ? gameData.title : ""
            font {
                bold: true
                pixelSize: 24
                capitalization: Font.SmallCaps
                family: "Roboto"
            }
        }
        Text {
            width: parent.width
            height: 300

            color: "#eee"
            text: gameData ? gameData.description : ""
            wrapMode: Text.WordWrap
            elide: Text.ElideRight
            horizontalAlignment: Text.AlignJustify
            font {
                pixelSize: 16
                family: "Roboto"
            }
        }
        Text {
            width: parent.width

            color: "#eee"
            text: gameData ? "Developer: " + gameData.developer : ""
            wrapMode: Text.WordWrap

            font {
                pixelSize: 16
                family: "Roboto"
            }
        }
    }
}
