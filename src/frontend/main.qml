import QtQuick 2.6
import QtQuick.Window 2.2
import QtGamepad 1.0


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

    // enable gamepad support
    Gamepad {
        id: gamepad1
        deviceId: GamepadManager.connectedGamepads.length > 0
                  ? GamepadManager.connectedGamepads[0]
                  : -1
    }
    Connections {
        target: GamepadManager
        onGamepadConnected: gamepad1.deviceId = deviceId
    }
    GamepadKeyNavigation {
        id: gamepadKeyNav
        gamepad: gamepad1
        active: true
        buttonAKey: Qt.Key_Return
        buttonL1Key: Qt.Key_A
        buttonL2Key: Qt.Key_A
        buttonR1Key: Qt.Key_D
        buttonR2Key: Qt.Key_D
    }


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
