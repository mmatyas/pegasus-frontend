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


TestCase {
    name: ""
    when: windowShown

    readonly property int imgSize: 24

    Grid {
        id: grid
        columns: 2

        Image {
            source: "image://blurhash/LEHV6nWB2yk8pyoJadR*.7kCMdnj"
            width: imgSize
            height: imgSize
        }

        Image {
            source: "image://blurhash/LGF5]+Yk^6#M@-5c,1J5@[or[Q6."
            width: imgSize
            height: imgSize
        }

        Image {
            source: "image://blurhash/L6Pj0^i_.AyE_3t7t7R**0o#DgR4"
            width: imgSize
            height: imgSize
        }

        Image {
            source: "image://blurhash/LKO2?U%2Tw=w]~RBVZRi};RPxuwH"
            width: imgSize
            height: imgSize
        }
    }


    Image {
        id: qmlExpected
        source: "qrc:/expected.png"
    }


    function test_output() {
        // TODO: Fuzzy image comparison
        // For now, just test that it loads and renders without errors
    }
}
