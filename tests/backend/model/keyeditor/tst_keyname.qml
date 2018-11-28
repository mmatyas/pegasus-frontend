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
    name: "keynames"
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

    function test_letter() {
        keyClick(Qt.Key_X);
        var key = keys.accept[keys.accept.length - 1];

        compare(key.name(), "X");
    }

    function test_gamepad() {
        // NOTE: keyClick can't click unknown keys
        var found = false;
        for (var i = 0; i < keys.accept.length; i++) {
            if (keys.accept[i].name() === "Gamepad 0 (A)")
                found = true;
        }
        compare(found, true);
    }

    function test_modifier() {
        keyClick(Qt.Key_X, Qt.AltModifier);
        var key = keys.accept[keys.accept.length - 1];

        var expected = isMac ? "\u2325X" : "Alt+X";
        compare(key.name(), expected);
    }
}
