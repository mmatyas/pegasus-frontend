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
import QtMultimedia 5.8


Item {
    property var gameData: api.currentGame

    onGameDataChanged: {
        videoPreview.playlist.clear();
        videoDelay.restart();
    }

    visible: gameData

    Timer {
        // a small delay to avoid loading videos during scrolling
        id: videoDelay
        interval: 50
        onTriggered: {
            if (gameData && gameData.assets.videos.length > 0) {
                for (var i = 0; i < gameData.assets.videos.length; i++) {
                    if (gameData.assets.videos[i])
                        videoPreview.playlist.addItem(gameData.assets.videos[i]);
                }
                videoPreview.play();
            }
        }
    }

    Rectangle {
        color: "#e8202020"
        anchors.fill: parent

        Rectangle {
            color: "#333"
            width: vpx(3)
            height: parent.height
            anchors.left: parent.right
        }
    }

    Column {
        width: parent.width - vpx(16)
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: vpx(16)

        // padding
        Item {
            width: parent.width
            height: vpx(32)
        }

        Image {
            id: logo
            width: parent.width - vpx(8)
            height: parent.width * 0.35

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
            width: parent.width
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter

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

        // description
        Text {
            width: parent.width
            wrapMode: Text.WordWrap

            text: gameData ? (gameData.summary || gameData.description) : ""
            color: "#eee"
            font {
                pixelSize: vpx(16)
                family: globalFonts.sans
            }
            maximumLineCount: 4
            elide: Text.ElideRight

            topPadding: vpx(16)
            leftPadding: vpx(8)
            rightPadding: vpx(8)
            bottomPadding: vpx(16)

            visible: text
        }

        // video container
        Rectangle {
            color: "#000"
            border { color: "#444"; width: 1 }

            width: parent.width
            height: parent.width * 0.45
            radius: vpx(4)

            visible: gameData && (gameData.assets.videos.length || gameData.assets.screenshots.length)

            Video {
                id: videoPreview
                visible: playlist.itemCount > 0

                anchors { fill: parent; margins: 1 }
                fillMode: VideoOutput.PreserveAspectFit

                autoPlay: true
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
