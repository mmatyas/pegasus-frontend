import QtQuick 2.0

Item {
    property variant gameData

    Column {
        width: parent.width
        height: parent.height
        spacing: 16

        Image {
            width: parent.width
            height: 192

            source: "file:" + gameData.assets.logo
            sourceSize { width: 512; height: 192 }

            fillMode: Image.PreserveAspectFit
        }

        Text {
            width: parent.width

            color: "#eee"
            text: gameData.title
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
            text: gameData.description
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
            text: "Developer: " + gameData.developer
            wrapMode: Text.WordWrap

            font {
                pixelSize: 16
                family: "Roboto"
            }
        }
    }
}
