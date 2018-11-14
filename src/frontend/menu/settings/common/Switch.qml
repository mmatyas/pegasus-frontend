// Pegasus Frontend
// Copyright (C) 2017  Mátyás Mustoha
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


import QtQuick 2.7


Item {
    id: root

    property bool checked: false

    width: height * 2


    function toggle() {
        checked = !checked;
    }

    Keys.onPressed: {
        if (api.keys.isAccept(event) && !event.isAutoRepeat) {
            event.accepted = true;
            root.toggle();
        }
    }


    MouseArea {
        id: mouseArea

        anchors.fill: parent
        onClicked: toggle();
    }

    Rectangle {
        id: thumb

        width: parent.height
        height: parent.height
        radius: height * 0.5

        color: "#bbb"

        anchors.left: parent.left
    }

    Rectangle {
        id: track

        width: parent.width - (parent.height - height)
        height: parent.height * 0.5
        radius: height * 0.5

        color: "#bbb"
        opacity: 0.3

        anchors.centerIn: parent
    }


    states: State {
        name: "checked"; when: checked
        PropertyChanges { target: thumb; color: "#3aa" }
        PropertyChanges { target: track; color: "#3aa" }
        PropertyChanges { target: track; opacity: 0.5 }
        AnchorChanges {
            target: thumb
            anchors.left: undefined
            anchors.right: parent.right
        }
    }

    transitions: [
        // NOTE: animation order seems to matter, which is why `reversible`
        // is not used here (it'd wait until the color is changed, and only
        // then would start changing the anchor)
        Transition {
            from: ""; to: "checked"
            AnchorAnimation { duration: 175 }
            ColorAnimation { duration: 175 }
        },
        Transition {
            from: "checked"; to: ""
            ColorAnimation { duration: 175 }
            AnchorAnimation { duration: 175 }
        }
    ]
}
