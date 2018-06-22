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


import QtQuick 2.9
import QtMultimedia 5.9


Item {
    property var gameData: api.currentGame

    onGameDataChanged: {
        videoPreview.stop();
        videoPreview.playlist.clear();
        videoDelay.restart();
    }

    // a small delay to avoid loading videos during scrolling
    Timer {
        id: videoDelay
        interval: 50
        onTriggered: {
            if (gameData && gameData.assets.videos.length > 0) {
                for (var i = 0; i < gameData.assets.videos.length; i++)
                    videoPreview.playlist.addItem(gameData.assets.videos[i]);

                videoPreview.play();
            }
        }
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

        visible: (gameData && (gameData.assets.videos.length || gameData.assets.screenshots.length)) || false

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
