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
    name: "replacing"
    when: windowShown

    property int oldKey


    Rectangle {
        id: rect
        width: 50; height: 50
        focus: true

        Keys.onPressed: keyEditor.replaceKey(0, oldKey, event);
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

    function test_sameEvent() {
        oldKey = Qt.Key_Return;
        var newKey = Qt.Key_X;
        var keyCount = keyEditor.keyCodesOf(0).length;
        compare(hasKey(0, oldKey), true);
        compare(hasKey(0, newKey), false);

        keyClick(newKey);

        tryCompare(changed, "count", 1);
        tryCompare(keyEditor.keyCodesOf(0), "length", keyCount);
        compare(hasKey(0, oldKey), false);
        compare(hasKey(0, newKey), true);
    }

    function test_differentEvent() {
        oldKey = Qt.Key_Return;
        var newKey = Qt.Key_Escape;
        var keyCount0 = keyEditor.keyCodesOf(0).length;
        var keyCount1 = keyEditor.keyCodesOf(1).length;
        compare(hasKey(0, oldKey), true);
        compare(hasKey(0, newKey), false);
        compare(hasKey(1, newKey), true);

        keyClick(newKey);

        tryCompare(changed, "count", 1);
        tryCompare(keyEditor.keyCodesOf(0), "length", keyCount0);
        tryCompare(keyEditor.keyCodesOf(1), "length", keyCount1 - 1);
        compare(hasKey(0, oldKey), false);
        compare(hasKey(0, newKey), true);
        compare(hasKey(1, newKey), false);
    }
}
