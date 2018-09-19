// Pegasus Frontend
// Copyright (C) 2017-2018  Mátyás Mustoha
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.


import "settings"
import "qrc:/qmlutils" as PegasusUtils
import QtQuick 2.8


FocusScope {
    id: root

    signal close

    function openScreen(url) {
        subscreen.source = url;
        subscreen.focus = true;
        root.state = "sub";
    }
    function openModal(url) {
        modal.source = url;
        modal.focus = true;
        root.state = "modal";
    }

    anchors.fill: parent
    enabled: focus
    visible: 0 < (x + width) && x < globalWidth


    SettingsMain {
        id: main
        focus: true
        anchors.right: parent.right

        onClose: root.close()
        onOpenKeySettings: root.openScreen("settings/KeyEditor.qml")
        onOpenGameDirSettings: root.openModal("settings/GameDirEditor.qml")
        onOpenProviderSettings: root.openModal("settings/ProviderEditor.qml")
    }


    Loader {
        id: modal
        asynchronous: true

        anchors.fill: parent

        enabled: focus
        onLoaded: item.focus = focus
        onFocusChanged: if (item) item.focus = focus
    }
    Connections {
        target: modal.item
        onClose: {
            main.focus = true;
            root.state = "";
        }
    }


    Loader {
        id: subscreen
        asynchronous: true

        width: parent.width
        height: parent.height
        anchors.left: main.right

        enabled: focus
        onLoaded: item.focus = focus
        onFocusChanged: if (item) item.focus = focus
    }
    Connections {
        target: subscreen.item
        onClose: {
            main.focus = true;
            root.state = "";
        }
    }


    states: [
        State {
            name: "sub"
            AnchorChanges {
                target: main
                anchors.right: subscreen.left
            }
            AnchorChanges {
                target: subscreen
                anchors.left: undefined
                anchors.right: parent.right
            }
        }
    ]

    // fancy easing curves, a la material design
    readonly property var bezierDecelerate: [ 0,0, 0.2,1, 1,1 ]
    readonly property var bezierSharp: [ 0.4,0, 0.6,1, 1,1 ]
    readonly property var bezierStandard: [ 0.4,0, 0.2,1, 1,1 ]

    transitions: [
        Transition {
            from: ""; to: "sub"
            AnchorAnimation {
                duration: 425
                easing { type: Easing.Bezier; bezierCurve: bezierStandard }
            }
        },
        Transition {
            from: "sub"; to: ""
            AnchorAnimation {
                duration: 400
                easing { type: Easing.Bezier; bezierCurve: bezierSharp }
            }
            onRunningChanged: if (!running) subscreen.source = ""
        }
    ]
}
