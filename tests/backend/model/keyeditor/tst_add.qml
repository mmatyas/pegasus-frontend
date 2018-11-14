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
import QtTest 1.11


TestCase {
    name: "adding"
    when: windowShown


    Rectangle {
        id: rect
        width: 50; height: 50
        focus: true

        Keys.onPressed: keyEditor.addKey(0, event);
    }

    SignalSpy {
        id: changed
        target: keyEditor
        signalName: "keysChanged"
    }


    function hasKey(event, key) {
        return keyEditor.keyCodesOf(event).indexOf(key) > -1;
    }

    function init() {
        keyEditor.resetKeys();
        changed.clear();
    }

    function test_addNew() {
        var test_key = Qt.Key_X;
        var key_count = keyEditor.keyCodesOf(0).length;
        compare(hasKey(0, test_key), false);

        keyClick(test_key);

        tryCompare(changed, "count", 1);
        tryCompare(keyEditor.keyCodesOf(0), "length", key_count + 1);
        compare(hasKey(0, test_key), true);
    }

    function test_addExisting() {
        var test_key = Qt.Key_Return;
        var key_count = keyEditor.keyCodesOf(0).length;
        compare(hasKey(0, test_key), true);

        keyClick(test_key);

        tryCompare(keyEditor.keyCodesOf(0), "length", key_count);
        compare(hasKey(0, test_key), true);
    }

    function test_addOverlapping() {
        var test_key = Qt.Key_Escape;
        var key_count_0 = keyEditor.keyCodesOf(0).length;
        var key_count_1 = keyEditor.keyCodesOf(1).length;
        compare(hasKey(0, test_key), false);
        compare(hasKey(1, test_key), true);

        keyClick(test_key);

        tryCompare(changed, "count", 1);
        tryCompare(keyEditor.keyCodesOf(0), "length", key_count_0 + 1);
        tryCompare(keyEditor.keyCodesOf(1), "length", key_count_1 - 1);
        compare(hasKey(0, test_key), true);
        compare(hasKey(1, test_key), false);
    }
}
