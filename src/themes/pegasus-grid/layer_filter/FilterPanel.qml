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
        radius: vpx(10)
    }

    Item {
        id: content

        property int normalTextSize: vpx(20)
        property int selectedIndex: 0
        property int padding: vpx(20)
        property int spacing: vpx(8)

        width: vpx(250)
        height: Math.min(vpx(600),
            padding * 2
            + itemHeader.height + spacing
            + itemTitleFilter.height + spacing
            + api.filters.count * (normalTextSize * 1.4 + spacing))


        Text {
            id: itemHeader
            text: qsTr("Filters")
            color: root.textColor
            font {
                bold: true
                pixelSize: vpx(26)
                family: globalFonts.sans
            }
            height: font.pixelSize * 1.5

            anchors {
                top: parent.top; topMargin: parent.padding
                left: parent.left; leftMargin: parent.padding
            }
        }

        TextLine {
            id: itemTitleFilter

            placeholder: qsTr("title")
            placeholderColor: "#bbb" // FIXME
            textColor: root.textColor
            fontSize: content.normalTextSize
            onTextChanged: api.filters.gameTitle = text

            focus: true
            KeyNavigation.down: itemFilterList

            anchors {
                top: itemHeader.bottom; topMargin: parent.spacing
                left: parent.left; leftMargin: parent.padding
                right: parent.right; rightMargin: parent.padding
            }
        }

        ListView {
            id: itemFilterList

            anchors {
                top: itemTitleFilter.bottom; topMargin: parent.spacing
                bottom: parent.bottom; bottomMargin: parent.padding * 0.75
                left: parent.left; leftMargin: parent.padding
                right: parent.right; rightMargin: parent.padding
            }
            clip: true

            model: api.filters.model
            delegate: CheckBox {
                text: modelData.name
                textColor: root.textColor
                fontSize: content.normalTextSize

                checked: modelData.enabled
                onCheckedChanged: api.filters.current.enabled = checked

            }
            spacing: parent.spacing

            Component.onCompleted: currentIndex = api.filters.index
            onCurrentIndexChanged: api.filters.index = currentIndex
        }
    }
}
