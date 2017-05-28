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


import QtQuick 2.0
import QtGamepad 1.0

FocusScope {
    property Gamepad gamepad

    signal exitUp()
    signal exitDown()

    width: fieldColumn.width
    height: fieldColumn.height

    Column {
        id: fieldColumn
        spacing: rpx(1)

        Rectangle {
            id: boxLabel
            color: "#444"
            width: label.width + rpx(10)
            height: rpx(20)
            anchors.right: parent.right

            Text {
                id: label
                text: qsTr("abxy")
                color: "#eee"
                font {
                    family: "Roboto"
                    pixelSize: parent.height
                    capitalization: Font.SmallCaps
                }
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.bottom
            }
        }

        GamepadConfigField {
            focus: true
            text: "a"
            textAlignRight: true
            anchors.right: parent.right

            Keys.onUpPressed: exitUp()
            KeyNavigation.down: fieldB
        }
        GamepadConfigField {
            id: fieldB
            text: "b"
            textAlignRight: true
            anchors { right: parent.right; rightMargin: rpx(3) }

            KeyNavigation.down: fieldX
        }
        GamepadConfigField {
            id: fieldX
            text: "x"
            textAlignRight: true
            anchors { right: parent.right; rightMargin: rpx(6) }
            KeyNavigation.down: fieldY
        }
        GamepadConfigField {
            id: fieldY
            text: "y"
            textAlignRight: true
            anchors { right: parent.right; rightMargin: rpx(9) }
            Keys.onDownPressed: exitDown()
        }
    }
}
