import QtQuick 2.0


Item {
    Image {
        anchors.fill: parent
        horizontalAlignment: Image.AlignLeft
        visible: currentGame

        asynchronous: true
        opacity: 0.35

        source: currentGame ? "file:" + currentGame.assets.screenshots[0] : ""
        sourceSize { width: 512; height: 512 }
        fillMode: Image.PreserveAspectFit
        smooth: false
    }

    Image {
        z: parent.z + 1
        anchors.fill: parent
        horizontalAlignment: Image.AlignLeft

        source: "/scanline.png"
        fillMode: Image.PreserveAspectFit
        smooth: false
    }
}
