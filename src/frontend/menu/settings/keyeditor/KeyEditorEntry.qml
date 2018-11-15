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


import QtQuick 2.6


FocusScope {
    id: root

    property int eventId

    signal editKey(int eventId, int keyCode)
    signal newKey(int eventId)
    signal delKey(int eventId, int keyCode)

    readonly property bool parentFocus: ListView.view.focus
    readonly property int textSize: vpx(18)
    readonly property int textBoxHeight: textSize * 2
    readonly property var eventNames: [
        qsTr("Accept/Select") + api.tr,
        qsTr("Cancel/Back") + api.tr,
        qsTr("Game details") + api.tr,
        qsTr("Filters") + api.tr,
        qsTr("Next page") + api.tr,
        qsTr("Previous page") + api.tr,
        qsTr("Scroll page up") + api.tr,
        qsTr("Scroll page down") + api.tr,
        qsTr("Main menu") + api.tr,
    ]

    width: ListView.view.width
    height: childrenRect.height

    function updateFocus() {
        if (keyList.hasKeys)
            keyList.focus = true;
        else
            addBtn.focus = true;
    }
    Component.onCompleted: updateFocus()


    Item {
        id: nameBox
        width: parent.width * 0.5 - keyList.spacing
        height: keyList.height + addBtn.height

        Rectangle {
            anchors.fill: parent
            color: "#444"
            opacity: (root.focus && root.parentFocus) ? 0.75 : 0.25
        }

        Text {
            id: nameLabel

            text: eventNames[eventId]
            color: "#eee"
            font.pixelSize: root.textSize
            font.family: globalFonts.sans
            font.bold: (root.focus && root.parentFocus)

            height: root.textBoxHeight
            verticalAlignment: Text.AlignVCenter
            leftPadding: vpx(8)
        }
    }

    ListView {
        id: keyList

        readonly property bool hasKeys: model.length
        property bool flagSetLastOnReload: false

        width: parent.width * 0.5
        height: model.length * (root.textBoxHeight + spacing)
        anchors.right: parent.right
        spacing: vpx(4)

        model: api.settings.keyEditor.keysOf(root.eventId)
        delegate: keyDelegate

        visible: hasKeys
        enabled: hasKeys
        KeyNavigation.down: addBtn

        onModelChanged: {
            if (flagSetLastOnReload) {
                currentIndex = count - 1;
                flagSetLastOnReload = false;
            }
            root.updateFocus();
        }
    }

    Component {
        id: keyDelegate

        FocusScope {
            id: keyButton
            width: ListView.view.width
            height: root.textBoxHeight

            onFocusChanged: editBtn.focus = true // do not remember relative focus

            KeyEditorEntryButton {
                id: editBtn

                height: parent.height
                anchors.left: parent.left
                anchors.right: delBtn.left
                anchors.rightMargin: keyList.spacing

                color: "#3bb"
                text: api.settings.keyEditor.keyName(modelData)
                onPressed: root.editKey(root.eventId, modelData.keyCode)

                focus: true
                KeyNavigation.right: delBtn
            }
            KeyEditorEntryButton {
                id: delBtn
                anchors.right: parent.right
                height: parent.height
                width: height * 0.75

                color: "#d88"
                text: "\u2212" // minus
                onPressed: root.delKey(root.eventId, modelData.keyCode);
            }
        }
    }

    KeyEditorEntryButton {
        id: addBtn

        readonly property real heightScale: keyList.hasKeys ? 0.75 : 1.0

        width: keyList.width - root.textBoxHeight * 0.75 - keyList.spacing
        height: root.textBoxHeight * heightScale
        anchors.top: keyList.bottom
        anchors.left: parent.horizontalCenter

        text: "+"
        color: "#3e8"
        onPressed: {
            keyList.flagSetLastOnReload = true;
            root.newKey(root.eventId);
        }

        KeyNavigation.up: keyList.hasKeys ? keyList : null
    }
}
