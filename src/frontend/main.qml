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

    FontLoader { id: font_loading; source: "/fonts/loading.ttf" }


    // wrapper to receive keyboard events
    Loader {
        id: theme_wrapper
        anchors.fill: parent
        focus: true
        asynchronous: true

        source: "/themes/pegasus-grid/theme.qml"
    }

    Rectangle {
        color: "#222"
        anchors.fill: parent

        visible: (theme_wrapper.status != Loader.Ready) || pegasus.isInitializing

        Text {
            text: "PEGASUS"
            color: "#333"
            font {
                pointSize: 192
                family: font_loading.name
            }
            anchors {
                bottom: parent.bottom
                left: parent.left
                leftMargin: 40
            }
        }
    }
}
