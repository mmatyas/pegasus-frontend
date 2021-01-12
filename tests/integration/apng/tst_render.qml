// Pegasus Frontend
// Copyright (C) 2017-2021  Mátyás Mustoha
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
import QtTest 1.11


Rectangle {
    id: root

    width: 200
    height: 100
    color: "cyan"

    AnimatedImage {
        id: actual
        source: "qrc:///actual.apng"
        anchors.left: parent.left
    }

    Image {
        id: expected
        source: "qrc:///expected.png"
        anchors.right: parent.right
    }


    TestCase {
        function test_animated() {
            verify(actual.frameCount > 12);
        }

        // NOTE: For some reason image grabbing produced incorrect
        // pixel values, so I couldn't add a comparison test yet
    }
}
