// Pegasus Frontend
// Copyright (C) 2017-2020  Mátyás Mustoha
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

    readonly property int imgSize: 24

    width: actual.width + expected.width
    height: actual.height
    color: "red"

    Grid {
        id: actual
        columns: 2

        Repeater {
            model: [
                "LEHV6nWB2yk8pyoJadR*.7kCMdnj",
                "LGF5]+Yk^6#M@-5c,1J5@[or[Q6.",
                "L6Pj0^i_.AyE_3t7t7R**0o#DgR4",
                "LKO2?U%2Tw=w]~RBVZRi};RPxuwH",
            ]
            delegate: Image {
                source: "image://blurhash/" + encodeURIComponent(modelData)
                width: imgSize
                height: imgSize
            }
        }
    }


    Image {
        id: expected
        source: "qrc:///expected.png"
        anchors.right: parent.right
    }


    TestCase {
        when: windowShown

        function test_render() {
            const actual_img = grabImage(actual);
            const expected_img = grabImage(expected);

            compare(actual_img.width, expected_img.width);
            compare(actual_img.height, expected_img.height);

            const max_diff = 255 * 0.05;
            for (let y = 0; y < actual_img.height; y++) {
                for (let x = 0; x < actual_img.width; x++) {
                    fuzzyCompare(actual_img.red(x, y), expected_img.red(x, y), max_diff);
                    fuzzyCompare(actual_img.green(x, y), expected_img.green(x, y), max_diff);
                    fuzzyCompare(actual_img.blue(x, y), expected_img.blue(x, y), max_diff);
                }
            }
        }
    }
}
