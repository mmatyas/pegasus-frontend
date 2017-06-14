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

/// This item provides an infinitely looping, autoscrolling view into
/// a taller content.
/// You can change the scrolling speed (pixels per second), and the
/// additional delay before and after the the animation.
/// If the content fits into the view, no scrolling happens.
Flickable {
    id: container

    property int scrollWaitDuration: 3000
    property int pixelsPerSecond: 26

    clip: true
    flickableDirection: Flickable.VerticalFlick
    contentWidth: width
    contentHeight: contentItem.childrenRect.height

    property int targetY: Math.max(contentHeight - height, 0);

    function recalcAnimation() {
        scrollAnimGroup.stop();
        contentY = 0;

        // the parameters of the sub-animations can't be properly
        // changed by regular binding while the group is running
        animScrollDown.to = targetY;
        animScrollDown.duration = (targetY / pixelsPerSecond) * 1000;
        animPauseHead.duration = scrollWaitDuration;
        animPauseTail.duration = scrollWaitDuration;

        scrollAnimGroup.restart();
    }
    onTargetYChanged: recalcAnimation()
    onScrollWaitDurationChanged: recalcAnimation()
    onPixelsPerSecondChanged: recalcAnimation()

    // cancel the animation on user interaction
    onMovementStarted: scrollAnimGroup.stop()

    SequentialAnimation {
        id: scrollAnimGroup
        running: true
        loops: Animation.Infinite

        PauseAnimation {
            id: animPauseHead
            duration: 0
        }
        NumberAnimation {
            id: animScrollDown
            target: container; property: "contentY"
            from: 0; to: 0; duration: 0
        }
        PauseAnimation {
            id: animPauseTail
            duration: 0
        }
        NumberAnimation {
            target: container; property: "contentY"
            to: 0; duration: 1000
        }
    }
}
