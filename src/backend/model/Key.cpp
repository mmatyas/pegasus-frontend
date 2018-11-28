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


#include "Key.h"

#include "AppSettings.h"

#include <QKeySequence>


namespace {
int get_modifiers(int keycode)
{
    return static_cast<int>(static_cast<unsigned>(keycode) & Qt::KeyboardModifierMask);
}
} // namespace


namespace model {

Key::Key(QObject* parent)
    : QObject(parent)
    , m_modifiers(Qt::NoModifier)
    , m_key(Qt::Key_unknown)
{}

Key::Key(const QKeySequence& keyseq, QObject* parent)
    : QObject(parent)
    , m_modifiers(keyseq.isEmpty() ? static_cast<int>(Qt::NoModifier) : get_modifiers(keyseq[0]))
    , m_key(keyseq.isEmpty() ? static_cast<int>(Qt::Key_unknown) : keyseq[0] - m_modifiers)
{}

// NOTE: toString was already taken...
QString Key::name() const
{
    const QKeySequence keyseq(keyCode());

    const auto gamepad_it = AppSettings::gamepadButtonNames.find(keyseq);
    if (gamepad_it != AppSettings::gamepadButtonNames.cend()) {
        return QStringLiteral("Gamepad %1 (%2)")
            .arg(QString::number(keyseq[0] - GamepadKeyId::A), gamepad_it->second);
    }

    return keyseq.toString(QKeySequence::NativeText);
}

} // namespace model
