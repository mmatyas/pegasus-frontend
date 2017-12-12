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
import QtQuick.Layouts 1.3
import QtMultimedia 5.8
import "qrc:/qmlutils" as PegasusUtils


Item {
    property var gameData: pegasus.collections.current.games.current

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

    ColumnLayout {
        anchors.fill: parent
        spacing: rpx(8)

        PegasusUtils.AutoScroll {
            Layout.fillWidth: true
            Layout.fillHeight: true

            Column {
                width: parent.width
                spacing: rpx(16)

                Image {
                    // logo
                    width: parent.width
                    height: parent.width * 0.4

                    asynchronous: true
                    source: gameData ? (gameData.assets.logo ? gameData.assets.logo : "") : ""
                    sourceSize { width: 512; height: 192 }
                    fillMode: Image.PreserveAspectFit
                }

                Text {
                    // title
                    color: "#eee"
                    text: gameData ? gameData.title : ""
                    width: parent.width
                    wrapMode: Text.WordWrap
                    horizontalAlignment: Text.AlignJustify
                    font {
                        bold: true
                        pixelSize: rpx(24)
                        capitalization: Font.SmallCaps
                        family: uiFont.name
                    }
                }

                Text {
                    // description
                    color: "#eee"
                    text: gameData ? gameData.description : ""
                    width: parent.width
                    wrapMode: Text.WordWrap
                    horizontalAlignment: Text.AlignJustify
                    font {
                        pixelSize: rpx(16)
                        family: uiFont.name
                    }
                }
            }
        }

        Rectangle {
            color: "#000"
            border { color: "#444"; width: 1 }

            Layout.fillWidth: true
            Layout.preferredHeight: parent.width * videoPreview.heightRatio
            Layout.bottomMargin: rpx(4)

            visible: gameData && gameData.assets.videos.length

            Video {
                id: videoPreview

                property real preferredHeightRatio: {
                    if (pegasus.currentCollection
                        && pegasus.currentCollection.tag === "steam")
                        return 0.5625; // 9/16

                    return 0.75;
                }
                property real heightRatio: metaData.resolution
                    ? Math.min(metaData.resolution.height / metaData.resolution.width,
                               preferredHeightRatio)
                    : preferredHeightRatio


                anchors { fill: parent; margins: 1 }
                fillMode: VideoOutput.PreserveAspectFit

                autoPlay: true
                playlist: Playlist {
                    playbackMode: Playlist.Loop
                }
            }
        }
    }
}
