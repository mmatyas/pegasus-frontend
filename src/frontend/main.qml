import QtQuick 2.6
import QtQuick.Window 2.2


Window {
    id: appWindow
    visible: true
    width: 1280
    height: 720
    title: "Pegasus"
    color: "#000"


    // wrapper to receive keyboard events
    Loader {
        anchors.fill: parent
        focus: true

        source: "/themes/pegasus-grid/theme.qml"
    }
}
