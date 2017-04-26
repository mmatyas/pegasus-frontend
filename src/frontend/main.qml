import QtQuick 2.6
import QtQuick.Window 2.2


Window {
    id: appWindow
    visible: true
    width: 1280
    height: 720
    title: "Pegasus"
    color: "#000"

    // provide relative pixel value calculation, for convenience
    property real winScale: Math.min(width / 1280.0, height / 720.0)
    function rpx(value) {
        return winScale * value;
    }

    // register custom global fonts here
    FontLoader { id: font_loading; source: "/fonts/loading.ttf" }


    // wrapper to receive keyboard events
    Item {
        focus: true
        anchors.fill: parent

        Keys.onEscapePressed: Qt.quit()
        Keys.forwardTo: theme_loader.item

        Loader {
            id: theme_loader
            anchors.fill: parent

            source: "/themes/pegasus-grid/theme.qml"
            asynchronous: true
        }
    }


    // loading screen
    Rectangle {
        color: "#222"
        anchors.fill: parent

        visible: (theme_loader.status != Loader.Ready) || pegasus.isInitializing

        Text {
            text: "PEGASUS"
            color: "#333"
            font {
                pixelSize: rpx(280)
                family: font_loading.name
            }
            anchors {
                bottom: parent.bottom
                left: parent.left
                leftMargin: rpx(56)
            }
        }
    }
}
