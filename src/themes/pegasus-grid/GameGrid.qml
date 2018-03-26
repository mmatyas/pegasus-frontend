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

    property var platformData: api.currentCollection

    model: platformData.gameList.model
    onModelChanged: { firstImageLoaded = false; cellHeightRatio = 0.5; }

    currentIndex: platformData.gameList.index
    onCurrentIndexChanged: api.currentCollection.gameList.index = currentIndex
    Component.onCompleted: positionViewAtIndex(currentIndex, GridView.Center)

    // For better visibility, box arts should be displayed in five columns if
    // the boxes are "tall", and four if they are "wide". There are two issues:
    //
    //   1. We don't want to hardcode the column count per platforms, so we need
    // a way to decide it in runtime. The problem is, because the images are
    // loaded asynchronously and individually, we don't know their dimensions!
    // Also technically images can have arbitrary sizes, that is, mixed tall and
    // wide images. As a workaround/heuristic, the first loaded image is used as
    // a base for further calculations.
    //
    //   2. GridView is too stupid to automatically set the cell dimensions,
    // we have to do it manually. Loop bindings and such also have to be avoided.

    property real columnCount: {
        if (cellHeightRatio > 1.2) return 5;
        if (cellHeightRatio > 0.6) return 4;
        return 3;
    }

    property bool firstImageLoaded: false
    property real cellHeightRatio: 0.5

    function calcHeightRatio(imageW, imageH) {
        cellHeightRatio = 0.5;

        if (imageW > 0 && imageH > 0)
            cellHeightRatio = imageH / imageW;
    }


    cellWidth: width / columnCount
    cellHeight: cellWidth * cellHeightRatio;


    Keys.onReleased: {
        if (event.key === Qt.Key_PageUp || event.key === Qt.Key_PageDown) {
            var rows_to_skip = Math.max(1, Math.round(grid_root.height / cellHeight));
            var games_to_skip = rows_to_skip * columnCount;
            if (event.key === Qt.Key_PageUp)
                currentIndex = Math.max(currentIndex - games_to_skip, 0);
            else
                currentIndex = Math.min(currentIndex + games_to_skip, model.length - 1);
        }
    }


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
        selected: GridView.isCurrentItem

        game: modelData

        onClicked: GridView.view.currentIndex = index
        imageHeightRatio: {
            if (firstImageLoaded) return cellHeightRatio;
            return 0.5;
        }


        onImageLoaded: {
            // NOTE: because images are loaded asynchronously,
            // firstImageLoaded may appear false multiple times!
            if (!firstImageLoaded) {
                firstImageLoaded = true;
                calcHeightRatio(imageWidth, imageHeight);
            }
        }
    }
}
