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


Rectangle {
    color: "#222"
    anchors.fill: parent

    Image {
        id: logo
        source: "/loading.svg"
        sourceSize.height: parent.height * 0.792
        opacity: 0.4

        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.margins: rpx(-5)
    }

    Item {
        anchors.left: parent.left
        anchors.leftMargin: parent.width * 0.66
        anchors.verticalCenter: parent.verticalCenter
        width: childrenRect.width
        height: childrenRect.height

        Text {
            id: counter

            text: pegasus.meta.gameCount
            color: "#eee"
            font {
                pixelSize: rpx(100)
                family: uiFont.name
            }
        }

        Text {
            text: qsTr("games found") + pegasus.tr
            color: "#aaa"
            font {
                pixelSize: rpx(33)
                family: uiFont.name
            }

            anchors.left: counter.left
            anchors.top: counter.baseline
            anchors.margins: rpx(5)
        }
    }
}
