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


import QtQuick 2.0


MouseArea {
    id: root

    property real startX: 0
    property real lastX: 0
    property bool active: false

    signal swipeLeft()
    signal swipeRight()

    onPressed: {
        startX = mouse.x;
        lastX = startX;
        active = true;
    }

    onPositionChanged: {
        if (!active)
            return;

        var diff = mouse.x - startX;

        if (diff < root.width * -0.5) {
            active = false;
            swipeLeft();
        }
        else if (root.width * 0.5 < diff) {
            active = false;
            swipeRight();
        }
    }
}
