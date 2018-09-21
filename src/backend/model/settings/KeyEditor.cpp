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


#include "KeyEditor.h"

#include <QKeySequence>
#include <QDebug>


namespace {
bool valid_event_id(int event_id)
{
    return 0 <= event_id && event_id < model::KeyEditor::eventCount();
}
} // namespace


namespace model {

KeyEditor::KeyEditor(QObject* parent)
    : QObject(parent)
    , m_button_names {
        { GamepadKeyId::A, QStringLiteral("A") },
        { GamepadKeyId::B, QStringLiteral("B") },
        { GamepadKeyId::X, QStringLiteral("X") },
        { GamepadKeyId::Y, QStringLiteral("Y") },
        { GamepadKeyId::L1, QStringLiteral("L1") },
        { GamepadKeyId::L2, QStringLiteral("L2") },
        { GamepadKeyId::L3, QStringLiteral("L3") },
        { GamepadKeyId::R1, QStringLiteral("R1") },
        { GamepadKeyId::R2, QStringLiteral("R2") },
        { GamepadKeyId::R3, QStringLiteral("R3") },
        { GamepadKeyId::SELECT, QStringLiteral("Select") },
        { GamepadKeyId::START, QStringLiteral("Start") },
        { GamepadKeyId::GUIDE, QStringLiteral("Guide") },
    }
{}

void KeyEditor::addKey(int event_id, int key)
{
    if (!valid_event_id(event_id))
        return;

    AppSettings::keys.add_key(static_cast<::KeyEvent>(event_id), key);
    AppSettings::save_config();
    emit keysChanged();
}

void KeyEditor::delKey(int event_id, int key)
{
    if (!valid_event_id(event_id))
        return;

    AppSettings::keys.del_key(static_cast<::KeyEvent>(event_id), key);
    AppSettings::save_config();
    emit keysChanged();
}

void KeyEditor::changeKey(int event_id, int old_key, int new_key)
{
    if (!valid_event_id(event_id))
        return;

    AppSettings::keys.del_key(static_cast<::KeyEvent>(event_id), old_key);
    AppSettings::keys.add_key(static_cast<::KeyEvent>(event_id), new_key);
    AppSettings::save_config();
    emit keysChanged();
}

void KeyEditor::resetKeys()
{
    AppSettings::keys.resetAll();
    AppSettings::save_config();
    emit keysChanged();
}

const QVector<int> KeyEditor::keysOf(int event_id) const
{
    if (!valid_event_id(event_id))
        return {};

    return AppSettings::keys.at(static_cast<::KeyEvent>(event_id));
}

const QString KeyEditor::keyName(int key) const
{
#ifndef Q_OS_MACOS
    // Qt workaround
    switch (key) {
        case Qt::Key_Shift:
            return QStringLiteral("Shift");
        case Qt::Key_Control:
            return QStringLiteral("Ctrl");
        case Qt::Key_Meta:
            return QStringLiteral("Meta");
        case Qt::Key_Alt:
            return QStringLiteral("Alt");
        case Qt::Key_AltGr:
            return QStringLiteral("AltGr");
        default:
            break;
    }
#endif
    if (m_button_names.count(key)) {
        return QStringLiteral("Gamepad %1 (%2)")
            .arg(QString::number(key - GamepadKeyId::A), m_button_names.at(key));
    }

    return QKeySequence(key).toString();
}

} // namespace model
