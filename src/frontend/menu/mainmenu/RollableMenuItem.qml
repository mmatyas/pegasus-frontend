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


import QtQuick 2.0


FocusScope {
    id: root

    property alias name: head.text
    property alias entries: submenu.children

    width: parent.width
    height: head.height

    onFocusChanged: {
        // when we leave the scope, reset
        if (!activeFocus) {
            head.focus = true;
            state = "";
        }
    }


    readonly property var bezierStandard: [ 0.4,0, 0.2,1, 1,1 ]
    Behavior on height {
        PropertyAnimation {
            duration: 200
            easing { type: Easing.Bezier; bezierCurve: bezierStandard }
        }
    }


    PrimaryMenuItem {
        id: head

        anchors.top: parent.top

        focus: true
        onActivated: {
            root.state = (root.state == "open") ? "" : "open";

            // if the head was selected using the mouse, we also
            // need to activate the parent
            if (!root.activeFocus)
                root.forceActiveFocus();

            if (activeFocus)
                submenuWrapper.focus = true;
            else
                focus = true;
        }
        onFocusChanged: {
            // on return from the submenu
            if (activeFocus)
                root.state = "";
        }

        selected: root.focus
    }

    FocusScope {
        id: submenuWrapper

        width: parent.width
        height: submenu.height

        visible: root.height > head.height
        anchors.bottom: parent.bottom

        Keys.onPressed: {
            if (api.keys.isCancel(event) && !event.isAutoRepeat) {
                event.accepted = true;
                head.focus = true;
            }
        }

        Column {
            id: submenu
            width: parent.width
        }
    }


    states: State {
        name: "open"
        PropertyChanges { target: root; height: head.height + submenu.height }
    }
}
