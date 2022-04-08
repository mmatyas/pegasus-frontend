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
import QtQuick 2.15
import QtQuick.Window 2.2
import Qt.labs.qmlmodels 1.0


FocusScope {
    id: root

    signal close
    signal openKeySettings
    signal openGamepadSettings
    signal openGameDirSettings
    signal openAndroidSafSettings
    signal openProviderSettings
    signal reloadRequested

    width: parent.width
    height: parent.height
    visible: 0 < (x + width) && x < Window.window.width

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


    readonly property list<SettingsEntry> optionList: [
        SettingsEntry {
            label: QT_TR_NOOP("Language")
            type: SettingsEntry.Type.Select
            selectBox: localeBox
            selectValue: Internal.settings.locales.currentName
            section: "general"
        },
        SettingsEntry {
            label: QT_TR_NOOP("Theme")
            type: SettingsEntry.Type.Select
            selectBox: themeBox
            selectValue: Internal.settings.themes.currentName
            section: "general"
        },
        SettingsEntry {
            label: QT_TR_NOOP("Fullscreen mode")
            desc: QT_TR_NOOP("On some platforms this setting may have no effect.")
            type: SettingsEntry.Type.Bool
            boolValue: Internal.settings.fullscreen
            boolSetter: (val) => Internal.settings.fullscreen = val
            section: "general"
        },

        SettingsEntry {
            label: QT_TR_NOOP("Change controls...")
            type: SettingsEntry.Type.Button
            buttonAction: root.openKeySettings
            section: "controls"
        },
        SettingsEntry {
            label: QT_TR_NOOP("Change gamepad layout...")
            type: SettingsEntry.Type.Button
            buttonAction: root.openGamepadSettings
            section: "controls"
        },
        SettingsEntry {
            label: QT_TR_NOOP("Enable mouse support")
            desc: QT_TR_NOOP("By default the cursor is visible if there are any pointer devices connected.")
            type: SettingsEntry.Type.Bool
            boolValue: Internal.settings.mouseSupport
            boolSetter: (val) => Internal.settings.mouseSupport = val
            section: "controls"
        },

        SettingsEntry {
            label: QT_TR_NOOP("Set game directories...")
            type: SettingsEntry.Type.Button
            buttonAction: root.openGameDirSettings
            section: "gaming"
        },
        SettingsEntry {
            label: QT_TR_NOOP("Accessible Android directories...")
            type: SettingsEntry.Type.Button
            buttonAction: root.openAndroidSafSettings
            section: "gaming"
            enabled: Qt.platform.os === "android"
        },
        SettingsEntry {
            label: QT_TR_NOOP("Only show existing games")
            desc: QT_TR_NOOP("Check the game files and only show games that actually exist. You can disable this to improve loading times.")
            type: SettingsEntry.Type.Bool
            boolValue: Internal.settings.verifyFiles
            boolSetter: (val) => Internal.settings.verifyFiles = val
            section: "gaming"
        },
        SettingsEntry {
            label: QT_TR_NOOP("Enable/disable data sources...")
            type: SettingsEntry.Type.Button
            buttonAction: root.openProviderSettings
            section: "gaming"
        },
        SettingsEntry {
            label: QT_TR_NOOP("Reload all games")
            type: SettingsEntry.Type.Button
            buttonAction: root.reloadRequested
            section: "gaming"
        }
    ]

    DelegateChooser {
        id: optionDelegates
        role: "type"

        DelegateChoice {
            roleValue: SettingsEntry.Type.Bool
            ToggleOption {
                label: qsTr(model.label) + api.tr
                desc: qsTr(model.desc) + api.tr
                checked: model.boolValue
                onCheckedChanged: model.boolSetter(checked)
            }
        }

        DelegateChoice {
            roleValue: SettingsEntry.Type.Button
            SimpleButton {
                label: qsTr(model.label) + api.tr
                onActivate: model.buttonAction()
                enabled: model.enabled
            }
        }

        DelegateChoice {
            roleValue: SettingsEntry.Type.Select
            MultivalueOption {
                label: qsTr(model.label) + api.tr
                value: model.selectValue
                onActivate: model.selectBox.focus = true
            }
        }
    }

    Component {
        id: sectionTitle
        SectionTitle {
            required property string section

            readonly property string trText: switch (section) {
                case "general": return QT_TR_NOOP("General");
                case "controls": return QT_TR_NOOP("Controls");
                case "gaming": return QT_TR_NOOP("Gaming");
            }

            text: qsTr(trText) + api.tr
        }
    }

    ListView {
        id: options
        model: optionList
        delegate: optionDelegates

        width: root.width * 0.7
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: header.bottom
        anchors.bottom: parent.bottom
        anchors.bottomMargin: header.height

        displayMarginBeginning: header.height
        displayMarginEnd: header.height

        focus: true
        highlightRangeMode: ListView.ApplyRange
        highlightMoveDuration: 150
        preferredHighlightBegin: height * 0.3
        preferredHighlightEnd: height * 0.7

        section.property: "section"
        section.delegate: sectionTitle
    }


    MultivalueBox {
        id: localeBox
        z: 3

        model: Internal.settings.locales
        index: Internal.settings.locales.currentIndex

        onClose: options.focus = true
        onSelect: Internal.settings.locales.currentIndex = index
    }
    MultivalueBox {
        id: themeBox
        z: 3

        model: Internal.settings.themes
        index: Internal.settings.themes.currentIndex

        onClose: options.focus = true
        onSelect: Internal.settings.themes.currentIndex = index
    }
}
