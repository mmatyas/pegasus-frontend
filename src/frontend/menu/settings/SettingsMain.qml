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


import "common"
import "qrc:/qmlutils" as PegasusUtils
import QtQuick 2.0


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
        if (api.keys.isCancel(event) && !event.isAutoRepeat) {
            event.accepted = true;
            root.close();
        }
    }


    PegasusUtils.HorizontalSwipeArea {
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
        z: 2
    }

    Flickable {
        id: container

        width: content.width
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.bottom: parent.bottom

        contentWidth: content.width
        contentHeight: content.height

        Behavior on contentY { PropertyAnimation { duration: 100 } }

        readonly property int yBreakpoint: height * 0.5
        readonly property int maxContentY: contentHeight - height

        function onFocus(item) {
            if (item.focus)
                contentY = Math.min(Math.max(0, item.y - yBreakpoint), maxContentY);
        }

        FocusScope {
            id: content

            focus: true
            enabled: focus

            width: contentColumn.width
            height: contentColumn.height

            Column {
                id: contentColumn
                spacing: vpx(5)

                width: root.width * 0.7
                height: implicitHeight

                Item {
                    width: parent.width
                    height: header.height + vpx(25)
                }

                SectionTitle {
                    text: qsTr("General") + api.tr
                    first: true
                }

                MultivalueOption {
                    id: optLanguage

                    focus: true

                    label: qsTr("Language") + api.tr
                    value: api.internal.settings.locales.currentName

                    onActivate: {
                        focus = true;
                        localeBox.focus = true;
                    }
                    onFocusChanged: container.onFocus(this)

                    KeyNavigation.down: optTheme
                }

                MultivalueOption {
                    id: optTheme

                    label: qsTr("Theme") + api.tr
                    value: api.internal.settings.themes.currentName

                    onActivate: {
                        focus = true;
                        themeBox.focus = true;
                    }
                    onFocusChanged: container.onFocus(this)

                    KeyNavigation.down: optFullscreen
                }

                ToggleOption {
                    id: optFullscreen

                    label: qsTr("Fullscreen mode") + api.tr
                    note: qsTr("On some platforms this setting may have no effect.") + api.tr

                    checked: api.internal.settings.fullscreen
                    onCheckedChanged: {
                        focus = true;
                        api.internal.settings.fullscreen = checked;
                    }
                    onFocusChanged: container.onFocus(this)

                    KeyNavigation.down: optKeyboardConfig
                }

                SectionTitle {
                    text: qsTr("Controls") + api.tr
                }

                SimpleButton {
                    id: optKeyboardConfig

                    label: qsTr("Change controls...") + api.tr
                    onActivate: {
                        focus = true;
                        root.openKeySettings();
                    }
                    onFocusChanged: container.onFocus(this)

                    KeyNavigation.down: optGamepadConfig
                }

                SimpleButton {
                    id: optGamepadConfig

                    label: qsTr("Change gamepad layout...") + api.tr
                    onActivate: {
                        focus = true;
                        root.openGamepadSettings();
                    }
                    onFocusChanged: container.onFocus(this)

                    KeyNavigation.down: optHideMouse
                }

                ToggleOption {
                    id: optHideMouse

                    label: qsTr("Enable mouse support") + api.tr
                    note: qsTr("By default the cursor is visible if there are any pointer devices connected.") + api.tr

                    checked: api.internal.settings.mouseSupport
                    onCheckedChanged: {
                        focus = true;
                        api.internal.settings.mouseSupport = checked;
                    }
                    onFocusChanged: container.onFocus(this)

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
                    onFocusChanged: container.onFocus(this)

                    KeyNavigation.down: optEditProviders
                }
                SimpleButton {
                    id: optEditProviders

                    label: qsTr("Enable/disable data sources...") + api.tr
                    onActivate: {
                        focus = true;
                        root.openProviderSettings();
                    }
                    onFocusChanged: container.onFocus(this)
                }

                Item {
                    width: parent.width
                    height: vpx(25)
                }
            }
        }
    }


    MultivalueBox {
        id: localeBox
        z: 3

        model: api.internal.settings.locales
        index: api.internal.settings.locales.currentIndex

        onClose: content.focus = true
        onSelect: api.internal.settings.locales.currentIndex = index
    }
    MultivalueBox {
        id: themeBox
        z: 3

        model: api.internal.settings.themes
        index: api.internal.settings.themes.currentIndex

        onClose: content.focus = true
        onSelect: api.internal.settings.themes.currentIndex = index
    }
}
