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
import Qt.labs.folderlistmodel 2.2


FocusScope {
    id: root

    signal cancel
    signal pick(string dir_path)

    anchors.fill: parent

    property string currentPathStr: ""
    function setCurrentPathStr(path) {
        var begin = 0;
        var end = path.length;

        var filePrefix = "file://";
        if (path.startsWith(filePrefix))
            begin += filePrefix.length;

        if (path.endsWith("/."))
            end -= 2;

        if (path.endsWith("/..")) {
            var lastSlash = path.lastIndexOf("/");
            end = path.lastIndexOf("/", lastSlash - 1);
        }

        currentPathStr = path.substring(begin, end);
    }


    FolderListModel {
        id: folderModel

        showDirs: true
        showDirsFirst: true
        showDotAndDotDot: true
        showOnlyReadable: true

        nameFilters: [
            "collections.pegasus.txt",
            "collections.txt",
        ]

        Component.onCompleted: setCurrentPathStr(folder.toString())
        onFolderChanged: setCurrentPathStr(folder.toString())
    }


    Rectangle {
        anchors.fill: parent
        color: "#000"

        opacity: 0.75

        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.LeftButton | Qt.RightButton
            onClicked: root.cancel()
        }
    }


    Rectangle {
        height: parent.height * 0.95
        width: height * 1.5
        color: "#222"

        radius: vpx(8)
        //border.width: vpx(2)
        //border.color: "#ccc"

        anchors.centerIn: parent


        MouseArea {
            anchors.fill: parent
        }

        Text {
            id: info
            text: "Select a collection file below (collections.pegasus.txt or collections.txt).\n"
                + "The directory of the file will then be added to the list."
            color: "#ee8"
            font.family: globalFonts.sans
            font.pixelSize: vpx(18)
            lineHeight: 1.15

            anchors.top: parent.top
            width: parent.width
            padding: font.pixelSize

            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            wrapMode: Text.WordWrap
        }
        Rectangle {
            id: path
            width: parent.width
            height: pathText.height
            color: "#111"

            anchors.top: info.bottom

            Text {
                id: pathText
                text: root.currentPathStr
                width: parent.width

                color: "#bbb"
                font.family: globalFonts.sans
                font.pixelSize: vpx(20)

                lineHeight: 2.5
                verticalAlignment: Text.AlignVCenter
                leftPadding: font.pixelSize; rightPadding: leftPadding
                elide: Text.ElideMiddle
            }
        }
        Rectangle {
            width: parent.width
            anchors.top: path.bottom
            anchors.bottom: buttonHelp.top
            color: "#333"
            clip: true

            ListView {
                id: list
                anchors.fill: parent

                model: folderModel
                delegate: fileDelegate

                focus: true
                highlightRangeMode: ListView.ApplyRange
                preferredHighlightBegin: height * 0.5 - vpx(18) * 1.25
                preferredHighlightEnd: height * 0.5 + vpx(18) * 1.25
                highlightMoveDuration: 0
            }
        }
        Item {
            id: buttonHelp
            anchors.bottom: parent.bottom
            height: info.height
        }
    }

    Component {
        id: fileDelegate

        Rectangle {
            readonly property bool highlighted: ListView.isCurrentItem || mouseArea.containsMouse

            color: highlighted ? "#484848" : "transparent"
            width: parent.width
            height: label.height

            function pickItem() {
                if (!fileIsDir) {
                    root.pick(currentPathStr);
                    return;
                }

                if (fileName == ".")
                    return;

                if (fileName == "..") {
                    var path = folderModel.parentFolder;

                    var pathStr = path.toString();
                    if (pathStr.length === 0 || pathStr === "file:")
                        return;

                    folderModel.folder = path;
                    return;
                }

                folderModel.folder = fileURL;
            }

            Keys.onReturnPressed: pickItem()
            Keys.onEnterPressed: pickItem()

            Image {
                id: icon
                source: fileIsDir ? "qrc:/frontend/assets/dir-icon.png" : ""

                fillMode: Image.PreserveAspectFit
                height: parent.height * 0.8
                width: height

                anchors.left: parent.left
                anchors.leftMargin: parent.height * 0.5
                anchors.top: parent.top
                anchors.topMargin: parent.height * 0.06
            }

            Text {
                id: label
                text: fileName
                verticalAlignment: Text.AlignVCenter
                lineHeight: 2.5

                color: "#eee"
                font.bold: !fileIsDir
                font.family: globalFonts.sans
                font.pixelSize: vpx(18)

                anchors.left: icon.right
                anchors.right: parent.right
                leftPadding: parent.height * 0.25
                rightPadding: parent.height * 0.5
                elide: Text.ElideRight
            }

            MouseArea {
                id: mouseArea
                anchors.fill: parent
                hoverEnabled: true
                onClicked: pickItem()
            }
        }
    }
}
