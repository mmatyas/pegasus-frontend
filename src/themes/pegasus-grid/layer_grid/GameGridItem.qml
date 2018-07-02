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


import QtQuick 2.7


Item {
    id: root

    property bool selected: false
    property var game

    property alias imageWidth: boxFront.paintedWidth
    property alias imageHeight: boxFront.paintedHeight
    property real imageHeightRatio: 0.5


    height: width * imageHeightRatio

    signal clicked()
    signal doubleClicked()
    signal imageLoaded(int imgWidth, int imgHeight)

    scale: selected ? 1.20 : 1.0
    z: selected ? 3 : 1


    Behavior on scale { PropertyAnimation { duration: 150 } }

    Image {
        id: boxFront
        anchors { fill: parent; margins: vpx(5) }

        asynchronous: true
        visible: game.assets.boxFront

        source: game.assets.boxFront || ""
        sourceSize { width: 256; height: 256 }
        fillMode: Image.PreserveAspectFit

        onStatusChanged: if (status === Image.Ready) {
            imageHeightRatio = paintedHeight / paintedWidth;
            root.imageLoaded(paintedWidth, paintedHeight);
        }
    }

    Image {
        anchors.centerIn: parent

        visible: boxFront.status === Image.Loading
        source: "/common/loading-spinner.png"

        RotationAnimator on rotation {
            loops: Animator.Infinite;
            from: 0;
            to: 360;
            duration: 500
        }
    }

    Text {
        width: parent.width - vpx(64)
        anchors.centerIn: parent

        visible: !game.assets.boxFront

        text: game.title
        wrapMode: Text.Wrap
        horizontalAlignment: Text.AlignHCenter
        color: "#eee"
        font {
            pixelSize: vpx(16)
            family: globalFonts.sans
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: root.clicked()
        onDoubleClicked: root.doubleClicked()
    }
}
