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


import QtQuick 2.8
import QtGraphicalEffects 1.0
import "qrc:/qmlutils" as PegasusUtils


Item {
    property var gameData: api.currentGame

    onVisibleChanged: {
        if (visible)
            scrollArea.restartScroll();
        else
            scrollArea.stopScroll();
    }


    // description
    PegasusUtils.AutoScroll {
        id: scrollArea
        width: parent.width
        anchors.top: parent.top
        anchors.bottom: playtimes.top

        Text {
            color: "#eee"
            text: (gameData && gameData.description) || ""
            width: parent.width
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignJustify
            font {
                pixelSize: vpx(16)
                family: globalFonts.sans
            }
        }
    }

    Column {
        id: playtimes
        width: parent.width
        anchors.bottom: actionButtons.top

        topPadding: labelFontSize * 1.5
        bottomPadding: topPadding
        spacing: vpx(4)

        readonly property color labelColor: "#4ae";
        readonly property int labelSpacing: labelFontSize / 2
        readonly property int labelFontSize: vpx(17)

        Row {
            width: parent.width
            spacing: playtimes.labelSpacing

            Text {
                text: "last played:"
                width: parent.width * 0.5
                color: playtimes.labelColor
                font {
                    pixelSize: playtimes.labelFontSize
                    family: globalFonts.sans
                    capitalization: Font.AllUppercase
                }
                horizontalAlignment: Text.AlignRight
            }

            Text {
                text: {
                    if (isNaN(gameData.lastPlayed))
                        return "never";

                    var now = new Date();

                    var diffHours = (now.getTime() - gameData.lastPlayed.getTime()) / 1000 / 60 / 60;
                    if (diffHours < 24 && now.getDate() === gameData.lastPlayed.getDate())
                        return "today";

                    var diffDays = Math.round(diffHours / 24);
                    if (diffDays <= 1)
                        return "yesterday";

                    return diffDays + " days ago"
                }
                color: "#eee"
                font {
                    pixelSize: playtimes.labelFontSize
                    family: globalFonts.sans
                }
            }
        }

        Row {
            width: parent.width
            spacing: playtimes.labelSpacing

            Text {
                text: "play time:"
                width: parent.width * 0.5
                color: playtimes.labelColor;
                font {
                    pixelSize: playtimes.labelFontSize
                    family: globalFonts.sans
                    capitalization: Font.AllUppercase
                }
                horizontalAlignment: Text.AlignRight
            }

            Text {
                text: {
                    var minutes = Math.ceil(gameData.playTime / 60)
                    if (minutes <= 90)
                        return Math.round(minutes) + " minutes";

                    return parseFloat((minutes / 60).toFixed(1)) + " hours"
                }
                color: "#eee"
                font {
                    pixelSize: playtimes.labelFontSize
                    family: globalFonts.sans
                }
            }
        }
    }

    Column {
        id: actionButtons
        width: parent.width
        anchors.bottom: parent.bottom
        spacing: vpx(4)

        GamePanelButton {
            id: toggleFavBtn
            text: "Toggle favorite on/off" // FIXME: translate

            property bool isFavorite: (gameData && gameData.favorite) || false
            function toggleFav() {
                if (api.currentGame)
                    api.currentGame.favorite = !api.currentGame.favorite;
            }

            KeyNavigation.up: launchBtn
            KeyNavigation.down: launchBtn
            Keys.onPressed: {
                if (api.keys.isAccept(event.key) && !event.isAutoRepeat) {
                    event.accepted = true;
                    toggleFav();
                }
            }
            onClicked: {
                focus = true;
                toggleFav();
            }

            Image {
                id: favHeart
                source: (parent.isFavorite && "../assets/icons/heart_filled.svg") || "../assets/icons/heart_empty.svg"
                sourceSize { width: 32; height: 32 }
                asynchronous: true
                fillMode: Image.PreserveAspectFit

                width: vpx(22)
                height: width
                anchors.left: parent.left
                anchors.leftMargin: (parent.height - height)
                anchors.verticalCenter: parent.verticalCenter

                visible: false
            }

            ColorOverlay {
                anchors.fill: favHeart
                source: favHeart
                color: parent.focus ? "#eee" : "#666"
            }
        }
        GamePanelButton {
            id: launchBtn
            text: "Launch"
            lineHeight: 2.5

            focus: true
            Keys.onPressed: {
                if (api.keys.isAccept(event.key) && !event.isAutoRepeat) {
                    event.accepted = true;
                    api.currentGame.launch();
                }
            }
            onClicked: {
                focus = true;
                api.currentGame.launch();
            }
        }
    }
}
