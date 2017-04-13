import QtQuick 2.0
import QtGraphicalEffects 1.0


Item {
    property var gameData: pegasus.currentGame

    Image {
        anchors.fill: parent
        horizontalAlignment: Image.AlignLeft
        visible: gameData

        asynchronous: true
        opacity: 0.35

        source: gameData ? "file:" + gameData.assets.screenshots[0] : ""
        sourceSize { width: 512; height: 512 }
        fillMode: Image.PreserveAspectFit
        smooth: false
    }

    LinearGradient {
        z: parent.z + 1
        width: parent.width * 0.75
        anchors {
            top: parent.top
            right: parent.right
            bottom: parent.bottom
        }
        start: Qt.point(0, 0)
        end: Qt.point(width, 0)
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#00000000" }
            GradientStop { position: 0.5; color: "#FF000000" }
        }
    }
}
