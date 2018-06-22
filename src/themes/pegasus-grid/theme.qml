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
import "layer_filter"
import "layer_gameinfo"
import "layer_grid"
import "layer_platform"


FocusScope {
    PlatformBar {
        id: topbar
        z: 300
        anchors {
            top: parent.top;
            left: parent.left; right: parent.right
        }
    }

    BackgroundImage {
        anchors {
            top: topbar.bottom; bottom: parent.bottom
            left: parent.left; right: parent.right
        }
    }

    GameGrid {
        id: gamegrid

        focus: true

        gridWidth: (parent.width * 0.6) - vpx(32)
        gridMarginTop: vpx(32)
        gridMarginRight: vpx(6)
        anchors {
            top: topbar.bottom; bottom: parent.bottom
            left: parent.left; right: parent.right
        }

        onLaunchRequested: api.currentGame.launch()
        onDetailsRequested: gamepreview.focus = true
        onFiltersRequested: filter.focus = true
        onNextPlatformRequested: topbar.next()
        onPrevPlatformRequested: topbar.prev()
    }

    GamePreview {
        id: gamepreview

        panelWidth: parent.width * 0.7 + vpx(72)
        anchors {
            top: topbar.bottom; bottom: parent.bottom
            left: parent.left; right: parent.right
        }

        onOpenRequested: gamepreview.focus = true
        onCloseRequested: gamegrid.focus = true
        onFiltersRequested: filter.focus = true
    }

    FilterLayer {
        id: filter
        anchors.fill: parent

        onCloseRequested: gamegrid.focus = true;
    }
}
