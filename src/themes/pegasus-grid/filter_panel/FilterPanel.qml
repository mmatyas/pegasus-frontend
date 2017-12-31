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


import QtQuick 2.7
import QtGraphicalEffects 1.0


FocusScope {
    id: root

    property alias panelColor: panel.color
    property color textColor: "#eee"

    width: content.width
    height: content.height

    Rectangle {
        id: panel
        color: "#ff6235"
        anchors.fill: parent
    }

    DropShadow {
        source: panel
        anchors.fill: panel
        radius: rpx(10)
    }

    Column {
        id: content

        property int normalTextSize: rpx(20)
        property int selectedIndex: 0

        padding: rpx(20)
        spacing: rpx(8)

        Text {
            text: qsTr("Filters")
            color: root.textColor
            font {
                bold: true
                pixelSize: rpx(26)
                family: uiFont.name
            }
            height: font.pixelSize * 1.5
        }

        TextLine {
            id: itemTitle

            placeholder: qsTr("title")
            placeholderColor: "#bbb" // FIXME
            textColor: root.textColor
            fontSize: content.normalTextSize
            onTextChanged: api.filters.title = text

            width: rpx(200)

            focus: true
            KeyNavigation.down: itemFavorites
        }

        CheckBox {
            id: itemFavorites

            text: qsTr("Favorites")
            textColor: root.textColor
            fontSize: content.normalTextSize

            checked: api.filters.favorite
            onCheckedChanged: api.filters.favorite = checked

            KeyNavigation.down: itemMultiplayer
        }

        CheckBox {
            id: itemMultiplayer

            text: qsTr("Multiplayer")
            textColor: root.textColor
            fontSize: content.normalTextSize

            checked: api.filters.playerCount > 1
            onCheckedChanged: api.filters.playerCount = (checked ? 2 : 1)
        }
    }
}
