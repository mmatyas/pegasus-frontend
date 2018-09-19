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


import QtQuick 2.0
import "qrc:/qmlutils" as PegasusUtils


FocusScope {
    id: root

    signal close
    signal openKeySettings
    signal openGamepadSettings
    signal openGameDirSettings
    signal openProviderSettings

    width: parent.width
    height: parent.height
    visible: 0 < (x + width) && x < globalWidth

    enabled: focus

    Keys.onPressed: {
        if (event.isAutoRepeat)
            return;

        if (api.keys.isCancel(event.key)) {
            event.accepted = true;
            root.close();
        }
    }


    PegasusUtils.HorizSwipeArea {
        anchors.fill: parent
        onSwipeRight: root.close()
    }

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.RightButton
        onClicked: root.close()
    }

    ScreenHeader {
        id: header
        text: qsTr("Settings") + api.tr
    }

    FocusScope {
        id: content

        focus: true
        enabled: focus

        anchors.top: header.bottom
        anchors.topMargin: vpx(30)
        anchors.horizontalCenter: parent.horizontalCenter
        width: parent.width * 0.7


        Column {
            anchors.fill: parent
            spacing: vpx(5)

            SectionTitle {
                text: qsTr("General") + api.tr
                first: true
            }

            MultivalueOption {
                id: optLanguage

                focus: true

                label: qsTr("Language") + api.tr
                value: api.settings.locales.current.name

                onActivate: {
                    focus = true;
                    localeBox.focus = true;
                }

                KeyNavigation.down: optTheme
            }

            MultivalueOption {
                id: optTheme

                label: qsTr("Theme") + api.tr
                value: api.settings.themes.current.name

                onActivate: {
                    focus = true;
                    themeBox.focus = true;
                }

                KeyNavigation.down: optFullscreen
            }

            ToggleOption {
                id: optFullscreen

                label: qsTr("Fullscreen mode") + api.tr
                note: qsTr("On some platforms this setting may have no effect") + api.tr

                checked: api.settings.fullscreen
                onCheckedChanged: {
                    focus = true;
                    api.settings.fullscreen = checked;
                }

                KeyNavigation.down: optKeyboardConfig
            }

            SectionTitle {
                text: qsTr("Controls") + api.tr
            }

            SimpleButton {
                id: optKeyboardConfig

                label: qsTr("Edit keyboard mapping...") + api.tr
                onActivate: {
                    focus = true;
                    root.openKeySettings();
                }

                KeyNavigation.down: optEditGameDirs
            }

            SectionTitle {
                text: qsTr("Gaming") + api.tr
            }
            SimpleButton {
                id: optEditGameDirs

                label: qsTr("Set game directories...") + api.tr
                onActivate: {
                    focus = true;
                    root.openGameDirSettings();
                }

                KeyNavigation.down: optEditProviders
            }
            SimpleButton {
                id: optEditProviders

                label: qsTr("Enable/disable data sources...") + api.tr
                onActivate: {
                    focus = true;
                    root.openProviderSettings();
                }
            }
        }
    }


    MultivalueBox {
        id: localeBox

        model: api.settings.locales.model
        index: api.settings.locales.index

        onClose: content.focus = true
        onSelect: api.settings.locales.index = index
    }
    MultivalueBox {
        id: themeBox

        model: api.settings.themes.model
        index: api.settings.themes.index

        onClose: content.focus = true
        onSelect: api.settings.themes.index = index
    }
}
