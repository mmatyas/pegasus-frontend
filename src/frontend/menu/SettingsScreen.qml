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


import "settings"
import "qrc:/qmlutils" as PegasusUtils
import QtQuick 2.8


MenuScreen {
    id: root

    Keys.onEscapePressed: close()


    PegasusUtils.HorizSwipeArea {
        anchors.fill: parent
        onSwipeRight: close()
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
                onActivate: keyboardEditor.focus = true

                KeyNavigation.down: optEditGameDirs
            }

            SectionTitle {
                text: qsTr("Gaming") + api.tr
            }
            SimpleButton {
                id: optEditGameDirs

                label: qsTr("Set game directories...") + api.tr
                onActivate: gameDirEditor.focus = true
            }
            SimpleButton {
                id: optEditProviders

                label: qsTr("Enable/disable data sources...") + api.tr
                onActivate: providerEditor.focus = true
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

    GameDirEditor {
        id: gameDirEditor
        onClose: content.focus = true
    }
    ProviderEditor {
        id: providerEditor
        onClose: content.focus = true
    }
    KeyEditor {
        id: keyboardEditor
        onClose: content.focus = true
    }
}
