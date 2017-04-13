import QtQuick 2.0
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

    Column {
        width: parent.width
        height: parent.height
        spacing: 16

        visible: gameData

        Image {
            // logo
            width: parent.width
            height: width * 0.4

            source: gameData ? (gameData.assets.logo ? "file:" + gameData.assets.logo : "") : ""
            sourceSize { width: 512; height: 192 }
            fillMode: Image.PreserveAspectFit
        }
        Text {
            // title
            width: parent.width

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
            width: parent.width

            color: "#eee"
            text: gameData ? gameData.description : ""
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignJustify
            elide: Text.ElideRight
            maximumLineCount: 5
            font {
                pixelSize: 16
                family: "Roboto"
            }
        }
        Video {
            id: videoPreview

            width: parent.width
            height: width * 0.75
            fillMode: VideoOutput.PreserveAspectFit

            playlist: Playlist {
                playbackMode: Playlist.Loop
            }
        }
    }
}
