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


import QtQuick 2.8
import QtGraphicalEffects 1.0

Item {
    property var gameData: api.currentGame

    Image {
        anchors.fill: parent
        visible: gameData

        asynchronous: true
        opacity: (gameData && gameData.assets.background) ? 1 : 0.35

        source: gameData ? gameData.assets.background || gameData.assets.screenshots[0] || "" : ""
        sourceSize { width: 512; height: 512 }
        fillMode: Image.PreserveAspectCrop
        smooth: false
    }

    LinearGradient {
        z: parent.z + 1
        width: parent.width * 0.75
        anchors {
            top: parent.top
            right: parent.right
            bottom: parent.bottom
        }
        start: Qt.point(0, 0)
        end: Qt.point(width, 0)
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#00000000" }
            GradientStop { position: 0.5; color: "#cc000000" }
        }
    }
}
