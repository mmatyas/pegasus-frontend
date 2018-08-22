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


import "help"
import "qrc:/qmlutils" as PegasusUtils
import QtQuick 2.6


MenuScreen {
    id: root

    Keys.onEscapePressed: root.close()

    readonly property int bodyFontSize: vpx(18)


    PegasusUtils.HorizSwipeArea {
        anchors.fill: parent
        onSwipeRight: root.close()
    }


    Column {
        id: content

        width: Math.min(parent.width, parent.height * 1.1)
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.bottom: parent.bottom

        SectionTitle {
            text: qsTr("About") + api.tr
        }
        Column {
            width: parent.width
            topPadding: bodyFontSize * 0.75
            leftPadding: bodyFontSize

            Text {
                text: qsTr("pegasus-frontend, version <tt>%1</tt> (%2)")
                    .arg(api.meta.gitRevision)
                    .arg(api.meta.gitDate)
                    + api.tr
                color: "#eee"
                font.pixelSize: bodyFontSize
                font.family: globalFonts.sans

                height: bodyFontSize * 1.5
                verticalAlignment: Text.AlignVCenter
            }
            Text {
                text: "Mátyás Mustoha et al. 2017-2018"
                color: "#eee"
                font.pixelSize: bodyFontSize
                font.family: globalFonts.sans

                height: bodyFontSize * 1.5
                verticalAlignment: Text.AlignVCenter
            }
        }

        SectionTitle {
            text: qsTr("Help") + api.tr
        }
        Column {
            width: parent.width
            topPadding: bodyFontSize * 0.75
            leftPadding: bodyFontSize

            Text {
                text: qsTr("You can find the documentation and user guides on the following address:")
                    + api.tr
                color: "#eee"
                font.pixelSize: bodyFontSize
                font.family: globalFonts.sans

                height: bodyFontSize * 1.5
                verticalAlignment: Text.AlignVCenter
            }
            Item {
                width: parent.width
                height: bodyFontSize * 0.5
            }
            QrBox {
                url: "pegasus-frontend.org/docs"
                image: "qrc:/frontend/assets/qr-docs.png"
                anchors.horizontalCenter: parent.horizontalCenter
            }
        }
    }
}
