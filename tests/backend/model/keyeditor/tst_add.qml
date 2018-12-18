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
import "utils.js" as Utils


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


    function init() {
        keyEditor.resetKeys();
        changed.clear();
    }

    function test_addNew() {
        var test_key = Qt.Key_X;
        compare(Utils.hasKey(keys.accept, test_key), false);

        keyClick(test_key);

        tryCompare(changed, "count", 1);
        compare(Utils.hasKey(keys.accept, test_key), true);
    }

    function test_addNewWithModifers() {
        var test_key = Qt.Key_X;
        var test_modifier = Qt.ControlModifier;
        compare(Utils.hasKeyMod(keys.accept, test_key, test_modifier), false);

        keyClick(test_key, test_modifier);

        tryCompare(changed, "count", 1);
        compare(Utils.hasKeyMod(keys.accept, test_key, test_modifier), true);
    }

    function test_addExisting() {
        var test_key = Qt.Key_Return;
        compare(Utils.hasKey(keys.accept, test_key), true);

        // Test only that it works
        keyClick(test_key);
    }

    function test_addOverlapping() {
        var test_key = Qt.Key_Escape;
        compare(Utils.hasKey(keys.accept, test_key), false);
        compare(Utils.hasKey(keys.cancel, test_key), true);

        keyClick(test_key);

        tryCompare(changed, "count", 1);
        compare(Utils.hasKey(keys.accept, test_key), true);
        compare(Utils.hasKey(keys.cancel, test_key), false);
    }
}
