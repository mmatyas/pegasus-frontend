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


import QtQuick 2.8
import QtGraphicalEffects 1.0
import QtMultimedia 5.8
import "qrc:/qmlutils" as PegasusUtils


FocusScope {
    id: root

    property bool drawLeftside: false

    property var gameData: api.currentGame
    onGameDataChanged: {
        videoPreview.playlist.clear();
        videoDelay.restart();
    }

    signal openRequested()
    signal closeRequested()

    visible: gameData

    Timer {
        // a small delay to avoid loading videos during scrolling
        id: videoDelay
        interval: 50
        onTriggered: {
            if (gameData && gameData.assets.videos.length > 0) {
                videoPreview.stop();
                for (var i = 0; i < gameData.assets.videos.length; i++) {
                    if (gameData.assets.videos[i])
                        videoPreview.playlist.addItem(gameData.assets.videos[i]);
                }
                videoPreview.play();
            }
        }
    }

    PegasusUtils.HorizSwipeArea {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.left: drawLeftside ? parent.left : parent.horizontalCenter

        swipePercent: drawLeftside ? 0.25 : 0.5

        onSwipeRight: openRequested()
        onSwipeLeft: closeRequested()
    }

    Rectangle {
        color: "#111"
        anchors.fill: parent
        opacity: 0.85

        Rectangle {
            color: "#222"
            width: vpx(2)
            height: parent.height
            anchors.left: parent.right
        }
    }

    Item {
        id: leftside
        anchors {
            left: parent.left; leftMargin: vpx(16)
            right: parent.horizontalCenter; rightMargin: vpx(20)
            top: parent.top; topMargin: vpx(40)
            bottom: parent.bottom; bottomMargin: vpx(8)
        }

        visible: root.drawLeftside
        onVisibleChanged: if (!visible) scrollArea.stopScroll()

        PegasusUtils.AutoScroll {
            id: scrollArea
            width: parent.width
            anchors.top: parent.top
            anchors.bottom: actionButtons.top
            anchors.bottomMargin: vpx(16)

            Text {
                // description
                color: "#eee"
                text: (gameData && gameData.description) || ""
                horizontalAlignment: Text.AlignJustify
                width: parent.width
                wrapMode: Text.WordWrap
                font {
                    pixelSize: vpx(16)
                    family: globalFonts.sans
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
                text: "Toggle favorite on/off"

                function toggleFav() {
                    if (api.currentGame)
                        api.currentGame.favorite = !api.currentGame.favorite;
                }

                Keys.onUpPressed: launchBtn.focus = true
                Keys.onDownPressed: launchBtn.focus = true
                Keys.onReturnPressed: toggleFav()
                Keys.onEnterPressed: toggleFav()
                Keys.onSpacePressed: toggleFav()
                onClicked: {
                    focus = true;
                    toggleFav();
                }

                Image {
                    id: favHeart
                    source: (gameData && gameData.favorite && "assets/heart_filled.svg") || "assets/heart_empty.svg"
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
                Keys.onUpPressed: toggleFavBtn.focus = true
                Keys.onDownPressed: toggleFavBtn.focus = true
                Keys.onReturnPressed: api.currentGame.launch()
                Keys.onEnterPressed: api.currentGame.launch()
                onClicked: {
                    focus = true;
                    api.currentGame.launch();
                }
            }
        }
    }

    Item {
        id: rightside
        anchors {
            left: parent.horizontalCenter; leftMargin: vpx(20)
            right: parent.right; rightMargin: vpx(20)
            top: parent.top; topMargin: vpx(40)
            bottom: parent.bottom; bottomMargin: vpx(8)
        }

        Image {
            id: logo
            width: parent.width
            height: width * 0.35

            asynchronous: true
            source: (gameData && gameData.assets.logo) || ""
            sourceSize { width: 512; height: 192 }
            fillMode: Image.PreserveAspectFit

            // title
            Text {
                color: "#eee"
                text: (gameData && gameData.title) || ""

                width: parent.width * 0.8
                anchors.centerIn: parent
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter

                font {
                    bold: true
                    pixelSize: vpx(30)
                    capitalization: Font.SmallCaps
                    family: globalFonts.sans
                }

                visible: parent.status != Image.Ready && parent.status != Image.Loading
            }
        }

        // year -- developer / publisher -- players
        Text {
            id: releaseDetails
            width: parent.width
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter

            anchors.top: logo.bottom
            topPadding: vpx(16)
            bottomPadding: vpx(16)

            text: {
                var text_tmp = "";

                if (!gameData)
                    return text_tmp;

                if (gameData.year > 0)
                    text_tmp += gameData.year;
                if (gameData.developer) {
                    if (text_tmp)
                        text_tmp += " \u2014 ";

                    text_tmp += gameData.developer;
                    if (gameData.publisher && gameData.developer !== gameData.publisher)
                        text_tmp += " / " + gameData.publisher;
                }
                return text_tmp;
            }
            color: "#eee"
            font {
                pixelSize: vpx(18)
                family: globalFonts.sans
            }

            visible: text
        }

        Text {
            id: summary
            width: parent.width
            wrapMode: Text.WordWrap

            anchors.top: releaseDetails.bottom
            topPadding: vpx(20)
            bottomPadding: vpx(40)

            text: gameData ? (gameData.summary || gameData.description) : ""
            color: "#eee"
            font {
                pixelSize: vpx(16)
                family: globalFonts.sans
            }
            maximumLineCount: 4
            elide: Text.ElideRight

            visible: text
        }

        Rectangle {
            id: videoBox
            color: "#000"
            border { color: "#444"; width: 1 }

            anchors.top: summary.bottom
            anchors.bottom: parent.bottom

            width: parent.width
            radius: vpx(4)

            visible: gameData && (gameData.assets.videos.length || gameData.assets.screenshots.length)

            Video {
                id: videoPreview
                visible: playlist.itemCount > 0

                anchors { fill: parent; margins: 1 }
                fillMode: VideoOutput.PreserveAspectFit

                playlist: Playlist {
                    playbackMode: Playlist.Loop
                }
            }

            Image {
                visible: !videoPreview.visible

                anchors { fill: parent; margins: 1 }
                fillMode: Image.PreserveAspectFit

                source: (gameData && gameData.assets.screenshots.length && gameData.assets.screenshots[0]) || ""
                sourceSize { width: 512; height: 512 }
                asynchronous: true
            }
        }
    }
}
