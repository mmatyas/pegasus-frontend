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


FocusScope {
    id: root

    property alias gridWidth: grid.width
    property int gridMarginTop: 0
    property int gridMarginRight: 0
    property var platformData: api.currentCollection

    signal launchRequested
    signal detailsRequested
    signal filtersRequested
    signal nextPlatformRequested
    signal prevPlatformRequested

    Keys.onPressed: {
        if (event.isAutoRepeat)
            return;

        if (api.keys.isPrevPage(event.key)) {
            event.accepted = true;
            prevPlatformRequested();
            return;
        }
        if (api.keys.isNextPage(event.key)) {
            event.accepted = true;
            nextPlatformRequested();
            return;
        }
        if (api.keys.isDetails(event.key)) {
            event.accepted = true;
            detailsRequested();
            return;
        }
        if (api.keys.isFilters(event.key)) {
            event.accepted = true;
            filtersRequested();
            return;
        }
    }

    GridView {
        id: grid

        focus: true

        anchors {
            top: parent.top; topMargin: root.gridMarginTop
            right: parent.right; rightMargin: root.gridMarginRight
            bottom: parent.bottom
        }

        model: platformData ? platformData.games.model : []
        onModelChanged: { firstImageLoaded = false; cellHeightRatio = 0.5; }

        currentIndex: platformData ? platformData.games.index : 0
        onCurrentIndexChanged: if (api.currentCollection) api.currentCollection.games.index = currentIndex
        Component.onCompleted: positionViewAtIndex(currentIndex, GridView.Center)

        Keys.onPressed: {
            if (api.keys.isAccept(event.key) && !event.isAutoRepeat) {
                event.accepted = true;
                root.launchRequested()
            }
            if (api.keys.isPageUp(event.key) || api.keys.isPageDown(event.key)) {
                event.accepted = true;
                var rows_to_skip = Math.max(1, Math.round(grid.height / cellHeight));
                var games_to_skip = rows_to_skip * columnCount;
                if (api.keys.isPageUp(event.key))
                    currentIndex = Math.max(currentIndex - games_to_skip, 0);
                else
                    currentIndex = Math.min(currentIndex + games_to_skip, model.length - 1);
            }
            if (event.modifiers === Qt.AltModifier && event.text) {
                event.accepted = true;
                api.currentCollection.games.jumpToLetter(event.text);
            }
        }

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

        displayMarginBeginning: anchors.topMargin

        highlight: Rectangle {
            color: "#0074da"
            width: grid.cellWidth
            height: grid.cellHeight
            scale: 1.20
            z: 2
        }

        highlightMoveDuration: 0

        delegate: GameGridItem {
            width: GridView.view.cellWidth
            selected: GridView.isCurrentItem

            game: modelData

            onClicked: GridView.view.currentIndex = index
            onDoubleClicked: {
                GridView.view.currentIndex = index;
                root.detailsRequested();
            }

            imageHeightRatio: {
                if (grid.firstImageLoaded) return grid.cellHeightRatio;
                return 0.5;
            }
            onImageLoaded: {
                // NOTE: because images are loaded asynchronously,
                // firstImageLoaded may appear false multiple times!
                if (!grid.firstImageLoaded) {
                    grid.firstImageLoaded = true;
                    grid.calcHeightRatio(imageWidth, imageHeight);
                }
            }
        }
    }
}
