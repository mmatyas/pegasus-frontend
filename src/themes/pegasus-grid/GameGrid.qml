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
    property var tallPlatforms: ["nes"]
    property real columnCount: platformData
        ? (tallPlatforms.indexOf(platformData.shortName) < 0 ? 4 : 5)
        : 1

    model: platformData ? platformData.games : 0
    onCurrentIndexChanged: pegasus.currentPlatform.currentGameIndex = currentIndex;

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

    delegate: GameGridItem {
        width: GridView.view.cellWidth
        height: GridView.view.cellHeight
        selected: GridView.isCurrentItem

        game: modelData

        onClicked: GridView.view.currentIndex = index
        onImageLoaded: {
            var img_ratio = imgHeight / imgWidth;
            var cell_ratio = grid_root.cellHeight / grid_root.cellWidth;
            if (img_ratio < cell_ratio)
                grid_root.cellHeight = grid_root.cellWidth * img_ratio;
        }
    }
}
