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

GridView {
    id: grid_root

    property var platformData: pegasus.currentPlatform
    // TODO: make these customizable
    property real columnCount: platformData
                               ? (platformData.shortName === "nes" ? 5 : 4)
                               : 1

    model: platformData ? platformData.games : 0
    onCurrentIndexChanged: pegasus.currentGameIndex = currentIndex;

    cellWidth: width / columnCount

    // Because the images are loaded asynchronously, we don't know the row heights,
    // and we don't want to hardcode per-platform settings either. As such, we have
    // to update cellHeight when an image gets loaded, by trying to fit the image
    // first by width, then adjusting cellHeight based on the images w/h ratio.

    // this is the max allowed height
    cellHeight: cellWidth * 2
    // on platform change, reset the height to avoid getting smaller and smaller
    onModelChanged: cellHeight = cellWidth * 2


    displayMarginBeginning: anchors.topMargin

    highlight: Rectangle {
        color: "#0074da"
        width: cellWidth
        height: cellHeight
        scale: 1.20
        z: 2
    }

    highlightMoveDuration: 0

    delegate: Item {
        width: GridView.view.cellWidth
        height: GridView.view.cellHeight

        scale: GridView.isCurrentItem ? 1.20 : 1.0
        z: GridView.isCurrentItem ? 3 : 1

        Behavior on scale { PropertyAnimation { duration: 150 } }

        Image {
            id: boxFront
            anchors { fill: parent; margins: rpx(5) }

            asynchronous: true
            visible: model.assets.boxFront

            source: model.assets.boxFront ? "file:" + model.assets.boxFront : ""
            sourceSize { width: 256; height: 256 }
            fillMode: Image.PreserveAspectFit

            onStatusChanged: if (status === Image.Ready) {
                var img_ratio = paintedHeight / paintedWidth;
                var cell_ratio = grid_root.cellHeight / grid_root.cellWidth;
                if (img_ratio < cell_ratio)
                    grid_root.cellHeight = grid_root.cellWidth * img_ratio;
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
            width: parent.width - rpx(64)
            anchors.centerIn: parent

            visible: !model.assets.boxFront

            text: model.title
            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignHCenter
            color: "#eee"
            font {
                pixelSize: rpx(16)
                family: "Roboto"
            }
        }

        MouseArea {
            anchors.fill: parent
            onClicked: parent.GridView.view.currentIndex = index
        }
    }
}
