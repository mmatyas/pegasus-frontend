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
import "qrc:/qmlutils" as PegasusUtils

Rectangle {
    property var platformModel: api.collections.model

    color: "#333"
    height: vpx(54)

    function next() { platformPath.incrementCurrentIndex() }
    function prev() { platformPath.decrementCurrentIndex() }

    PathView {
        id: platformPath
        model: platformModel
        delegate: platformCardDelegate

        path: Path {
            startX: vpx(-400)
            startY: vpx(36)

            PathAttribute { name: "itemZ"; value: 201 }
            PathLine { x: parent.width * 0.2; y: vpx(36) }

            PathPercent { value: 0.04 }
            PathAttribute { name: "itemZ"; value: 200 }
            PathLine { x: parent.width * 0.23; y: vpx(18) }

            PathPercent { value: 0.08 }
            PathLine { x: parent.width * 0.7; y: vpx(18) }

            PathPercent { value: 1 }
            PathAttribute { name: "itemZ"; value: vpx(5) }
        }

        pathItemCount: 20

        snapMode: PathView.SnapOneItem
        preferredHighlightBegin: 0.04
        preferredHighlightEnd: 0.05

        Component.onCompleted: currentIndex = api.collections.index
        onCurrentIndexChanged: api.collections.index = currentIndex
    }

    Component {
        id: platformCardDelegate

        PlatformCard {
            platformShortName: shortName
            isOnTop: PathView.isCurrentItem

            visible: PathView.onPath

            z: PathView.itemZ
            width: parent.parent.width * 0.5
            height: vpx(72)
        }
    }

    MouseArea {
        anchors.fill: parent
        onWheel: {
            wheel.accepted = true;
            if (wheel.angleDelta.x > 0 || wheel.angleDelta.y > 0)
                platformPath.incrementCurrentIndex();
            else
                platformPath.decrementCurrentIndex();
        }
    }
    PegasusUtils.HorizontalSwipeArea {
        anchors.fill: parent
        onSwipeLeft: platformPath.incrementCurrentIndex()
        onSwipeRight: platformPath.decrementCurrentIndex()
    }
}
