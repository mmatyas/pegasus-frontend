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

Image {
    property string currentField: ""

    readonly property var fieldPoints: {
        'x': [0.7220015228339909, 0.37789453125],
        'y': [0.7961282330885671, 0.2662421875],
        'a': [0.7961282330885671, 0.48940625],
        'b': [0.8702523500493335, 0.37789453125],
        'start': [0.5852467670767555, 0.26057421875],
        'guide': [0.49821842011925055, 0.4911015625],
        'select': [0.41119525974936494, 0.2585859375],
        'lsb': [0.3374886630919495, 0.63964453125],
        'lsx': [0.3374886630919495, 0.63964453125],
        'lsy': [0.3374886630919495, 0.63964453125],
        'rsb': [0.6589481771465516, 0.63964453125],
        'rsx': [0.6589481771465516, 0.63964453125],
        'rsy': [0.6589481771465516, 0.63964453125],
        'lt': [0.17763803266611428, 0.0432109375],
        'lb': [0.21772516837543548, 0.12984765625],
        'rt': [0.8221260035301471, 0.0432109375],
        'rb': [0.7820907336970186, 0.12984765625],
        'dpup': [0.2009361842518655, 0.26973828125],
        'dpdown': [0.2009361842518655, 0.485796875],
        'dpleft': [0.12921605065262962, 0.377765625],
        'dpright': [0.2726537245572917, 0.377765625],
    }
    readonly property var verticalAxes: ['lsy', 'rsy']
    readonly property var horizontalAxes: ['lsx', 'rsx']

    readonly property double currentPointX: currentField ? fieldPoints[currentField][0] * width : 0
    readonly property double currentPointY: currentField ? fieldPoints[currentField][1] * height : 0
    readonly property bool currentFieldIsAxis: currentField && (horizontalAxes.includes(currentField) || verticalAxes.includes(currentField))


    source: "qrc:/frontend/assets/gamepad/preview.png"
    fillMode: Image.PreserveAspectFit


    Rectangle {
        id: selectionCenter

        color: "#3cc"
        width: vpx(20)
        height: width
        radius: width * 0.5

        visible: currentField
        x: currentPointX - radius
        y: currentPointY - radius
    }

    Rectangle {
        id: selectionAxis

        color: selectionCenter.color
        width: selectionCenter.width * 2
        height: vpx(3)

        visible: currentFieldIsAxis
        x: currentPointX - width / 2
        y: currentPointY - height / 2

        transformOrigin: Item.Center
        rotation: verticalAxes.includes(currentField) ? 90 : 0
    }
}
