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


    Item {
        id: container

        width: parent.width
        height: logo.height + counter.height + rpx(80)

        anchors.centerIn: parent

        Image {
            id: logo
            source: "/loading.svg"
            sourceSize.height: rpx(350)

            anchors.horizontalCenter: parent.horizontalCenter
        }

        Text {
            id: counter
            text: qsTr("%1 games found").arg(pegasus.meta.gameCount)

            color: "#eee"
            font {
                pixelSize: rpx(35)
                family: uiFontCondensed.name
            }

            anchors.bottom: parent.bottom
            anchors.horizontalCenter: parent.horizontalCenter
        }
    }
}
