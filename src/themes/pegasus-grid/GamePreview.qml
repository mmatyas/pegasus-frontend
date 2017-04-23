import QtQuick 2.0
import QtQuick.Layouts 1.3
import QtMultimedia 5.8


Item {
    property var gameData: pegasus.currentGame

    onGameDataChanged: {
        videoPreview.playlist.clear();
        if (gameData && gameData.assets.videos.length > 0) {
            for (var i = 0; i < gameData.assets.videos.length; i++) {
                if (gameData.assets.videos[i])
                    videoPreview.playlist.addItem("file:" + gameData.assets.videos[i]);
            }
            videoPreview.play();
        }
    }

    visible: gameData

    ColumnLayout {
        anchors.fill: parent
        spacing: 16

        Image {
            // logo
            Layout.fillWidth: true
            Layout.preferredHeight: parent.width * 0.4

            asynchronous: true
            source: gameData ? (gameData.assets.logo ? "file:" + gameData.assets.logo : "") : ""
            sourceSize { width: 512; height: 192 }
            fillMode: Image.PreserveAspectFit
        }

        Text {
            // title
            Layout.fillWidth: true

            color: "#eee"
            text: gameData ? gameData.title : ""
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignJustify
            font {
                bold: true
                pixelSize: 24
                capitalization: Font.SmallCaps
                family: "Roboto"
            }
        }

        Text {
            // description
            Layout.fillWidth: true
            Layout.fillHeight: true

            color: "#eee"
            text: gameData ? gameData.description : ""
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignJustify
            elide: Text.ElideRight
            font {
                pixelSize: 16
                family: "Roboto"
            }
        }

        Rectangle {
            color: "#000"
            border { color: "#444"; width: 1 }

            Layout.fillWidth: true
            Layout.preferredHeight: parent.width * 0.75
            Layout.bottomMargin: 4

            visible: videoPreview.playlist.itemCount

            Video {
                id: videoPreview

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
