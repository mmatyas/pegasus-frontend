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


import QtQuick 2.8

FocusScope {
    property alias label: name.text
    property alias model: list.model
    property alias index: list.currentIndex

    width: parent.width
    height: fontSize * 1.5
    property int fontSize: rpx(28)

    Text {
        id: name
        width: parent.width * 0.5
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter

        color: "#eee"
        font.pixelSize: fontSize
        font.family: uiFont.name
    }

    ListView {
        id: list
        height: parent.height
        anchors {
            left: parent.horizontalCenter; leftMargin: leftBracket.width
            right: parent.right; rightMargin: rightBracket.width
            verticalCenter: parent.verticalCenter
        }

        highlightRangeMode: ListView.StrictlyEnforceRange
        highlightMoveDuration: 200
        orientation: ListView.Horizontal
        clip: true
        focus: true

        delegate: Item {
            width: ListView.view.width
            height: ListView.view.height

            Text {
                id: listDelegateText
                anchors.centerIn: parent
                text: modelData.name

                color: "#eee"
                font.pixelSize: fontSize
                font.family: uiFont.name
            }
        }

        Keys.onLeftPressed: decrementCurrentIndex();
        Keys.onRightPressed: incrementCurrentIndex();
    }

    Text {
        id: leftBracket
        anchors.left: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        visible: parent.activeFocus && index > 0

        text: "\u2329"
        color: "#3aa"
        font.pixelSize: fontSize
        font.family: uiFont.name
        font.bold: true
    }
    Text {
        id: rightBracket
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        visible: parent.activeFocus && index < (model.length - 1)

        text: "\u232a"
        color: "#3aa"
        font.pixelSize: fontSize
        font.family: uiFont.name
        font.bold: true
    }
}
