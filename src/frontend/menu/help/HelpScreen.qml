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


import "../common"
import "qrc:/qmlutils" as PegasusUtils
import QtQuick 2.6


MenuScreen {
    id: root

    Keys.onPressed: event => {
        if (api.keys.isCancel(event) && !event.isAutoRepeat) {
            event.accepted = true;
            root.close();
        }
    }

    readonly property int bodyFontSize: vpx(18)


    PegasusUtils.HorizontalSwipeArea {
        anchors.fill: parent
        onSwipeRight: root.close()
    }


    Flickable {
        width: content.width
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.bottom: parent.bottom

        contentWidth: content.width
        contentHeight: content.height

        readonly property real maxContentY: Math.max(0, contentHeight - height)

        focus: true
        Keys.onUpPressed: if (0 < contentY) flick(0, 500)
        Keys.onDownPressed: if (contentY < maxContentY) flick(0, -500)

        Column {
            id: content
            width: vpx(800)

            Column {
                id: about
                width: parent.width

                SectionTitle {
                    text: qsTr("About") + api.tr
                }
                Text {
                    property string programInfo: qsTr("Pegasus Frontend, version <tt>%1</tt> (%2)")
                        .arg(Internal.meta.gitRevision)
                        .arg(Internal.meta.gitDate)
                        + api.tr

                    text: programInfo + "<br>Copyright \u00a9 2017-2020 Mátyás Mustoha"
                    color: "#eee"
                    font.pixelSize: bodyFontSize
                    font.family: globalFonts.sans
                    lineHeight: 1.25
                    padding: bodyFontSize
                }
            }

            Column {
                id: help
                width: parent.width

                SectionTitle {
                    text: qsTr("Help") + api.tr
                }
                Text {
                    text: qsTr("You can find the documentation and user guide on the following address:")
                        + api.tr
                    color: "#eee"
                    font.pixelSize: bodyFontSize
                    font.family: globalFonts.sans
                    lineHeight: 1.25
                    padding: bodyFontSize
                }
                QrBox {
                    url: "pegasus-frontend.org/docs"
                    image: "qrc:/frontend/assets/qr-docs.png"
                    anchors.horizontalCenter: parent.horizontalCenter
                }
                Item {
                    width: parent.width
                    height: bodyFontSize
                }
            }

            Column {
                id: licenses
                width: parent.width

                readonly property color textColor: "#ccc"
                readonly property int textSize: bodyFontSize * 0.85
                readonly property real textLineHeight: 1.2


                SectionTitle {
                    text: qsTr("Licenses") + api.tr
                }
                Column {
                    width: parent.width - bodyFontSize * 2
                    anchors.horizontalCenter: parent.horizontalCenter
                    topPadding: bodyFontSize
                    bottomPadding: licenses.textSize * 2
                    spacing: licenses.textSize * 0.9

                    Text {
                        text: "This program is free software: you can redistribute it and/or modify "
                            + "it under the terms of the GNU General Public License as published by "
                            + "the Free Software Foundation, either version 3 of the License, or "
                            + "(at your option) any later version."
                        color: licenses.textColor
                        lineHeight: licenses.textLineHeight
                        font.pixelSize: licenses.textSize
                        font.family: globalFonts.sans

                        width: parent.width
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignJustify
                    }
                    Text {
                        text: "This program is distributed in the hope that it will be useful, "
                            + "but WITHOUT ANY WARRANTY; without even the implied warranty of "
                            + "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the "
                            + "GNU General Public License for more details."
                        color: licenses.textColor
                        lineHeight: licenses.textLineHeight
                        font.pixelSize: licenses.textSize
                        font.family: globalFonts.sans

                        width: parent.width
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignJustify
                    }
                    Text {
                        text: "You should have received a copy of the full license along with this "
                            + "program. If not, you can also find it on the following address: "
                            + "<tt>http://pegasus-frontend.org/license</tt>."
                        color: licenses.textColor
                        lineHeight: licenses.textLineHeight
                        font.pixelSize: licenses.textSize
                        font.family: globalFonts.sans

                        width: parent.width
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignJustify
                    }
                }
                Row {
                    id: qt
                    width: parent.width
                    leftPadding: bodyFontSize * 1.3
                    rightPadding: leftPadding
                    topPadding: bodyFontSize * 0.15
                    bottomPadding: topPadding

                    Image {
                        source: "qrc:/frontend/assets/qt-logo.png"
                        width: vpx(40)
                        fillMode: Image.PreserveAspectFit
                    }
                    Text {
                        text: "Uses the Qt Toolkit. Copyright \u00a9 The Qt Company Ltd. All rights reserved."
                        color: licenses.textColor
                        font.pixelSize: licenses.textSize
                        font.family: globalFonts.sans

                        anchors.verticalCenter: parent.verticalCenter
                        leftPadding: bodyFontSize * 0.75
                    }
                }
                Row {
                    id: roboto
                    width: parent.width
                    leftPadding: qt.leftPadding
                    topPadding: qt.topPadding
                    bottomPadding: qt.bottomPadding
                    rightPadding: qt.rightPadding

                    Item {
                        width: vpx(40)
                        height: width

                        Text {
                            text: "Aa"
                            color: licenses.textColor
                            font.pixelSize: bodyFontSize * 1.7
                            font.family: globalFonts.sans
                            font.bold: true
                            anchors.centerIn: parent
                        }
                    }
                    Text {
                        text: "Uses Roboto fonts. Copyright \u00a9 Google Inc. All rights reserved."
                        color: licenses.textColor
                        font.pixelSize: licenses.textSize
                        font.family: globalFonts.sans

                        anchors.verticalCenter: parent.verticalCenter
                        leftPadding: bodyFontSize * 0.75
                    }
                }
                Text {
                    text: "All trademarks, service marks, trade names, trade dress, product names "
                        + "and logos are property of their respective owners. All company, product "
                        + "and service names used in this product are for identification purposes "
                        + "only. Use of these names, logos, and brands does not imply endorsement."
                    color: licenses.textColor
                    lineHeight: licenses.textLineHeight
                    font.pixelSize: licenses.textSize
                    font.family: globalFonts.sans

                    width: parent.width
                    wrapMode: Text.WordWrap
                    horizontalAlignment: Text.AlignJustify
                    padding: bodyFontSize
                    topPadding: licenses.textSize * 2
                }
            }
        }
    }
}
