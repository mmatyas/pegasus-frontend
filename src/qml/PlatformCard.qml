import QtQuick 2.6

Item {
    property alias platformShortName: label.text

    clip: true
    antialiasing: true

    Rectangle {
        id: main
        width: parent.width * 0.8
        height: parent.height
        color: "#ff4035"
    }

    Rectangle {
        width: main.height * 1.7
        height: width
        color: main.color
        rotation: -70
        anchors {
            verticalCenter: parent.top
            horizontalCenter: main.right
            horizontalCenterOffset: -27
        }

        Text {
            id: label
            width: 0
            text: ""
            color: "#eee"
            anchors {
                left: parent.left
                bottom: parent.bottom
                leftMargin: 8
                bottomMargin: 4
            }
            font {
                bold: true
                capitalization: Font.AllUppercase
                pixelSize: 16
                family: "Roboto Condensed"
            }
        }

        Rectangle {
            color: "#faa"
            width: parent.width
            height: 2
            anchors.top: parent.bottom
            anchors.left: parent.left
        }
    }

    Image {
        source: "/platform_logo/" + platformShortName + ".svg"
        sourceSize.height: 100
        width: parent.width * 0.6
        height: parent.height - 12
        fillMode: Image.PreserveAspectFit
        anchors {
            bottom: parent.bottom
            bottomMargin: 6
            horizontalCenter: parent.horizontalCenter
            horizontalCenterOffset: -32
        }
    }
}
