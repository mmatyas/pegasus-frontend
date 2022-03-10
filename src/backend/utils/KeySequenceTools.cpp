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


#include "KeySequenceTools.h"

#include "AppSettings.h"
#include "utils/FakeQKeyEvent.h"

#include <QKeyEvent>
#include <array>


namespace {
bool is_modifier(const int keycode)
{
    return keycode == Qt::Key_Control
        || keycode == Qt::Key_Shift
        || keycode == Qt::Key_Meta
        || keycode == Qt::Key_Alt;
}

int keymods_to_int(const Qt::KeyboardModifiers keymods)
{
    static constexpr std::array<Qt::KeyboardModifier, 4> possible_modifiers {
        Qt::KeyboardModifier::AltModifier,
        Qt::KeyboardModifier::ControlModifier,
        Qt::KeyboardModifier::MetaModifier,
        Qt::KeyboardModifier::ShiftModifier,
    };

    int result = 0;
    for (const auto modifier : possible_modifiers) {
        if (keymods & modifier)
            result |= modifier;
    }
    return result;
}
} // namespace


namespace utils {
QKeySequence qmlevent_to_keyseq(const QVariant& event_variant)
{
    static constexpr auto QML_KEYEVENT_CLASSNAME = "QQuickKeyEvent";

    const QObject* const event_qobj = event_variant.value<QObject*>();
    if (!event_qobj)
        return {};

    const char* const event_qobj_class = event_qobj->metaObject()->className();
    if (::strcmp(event_qobj_class, QML_KEYEVENT_CLASSNAME) != 0)
        return {};

    const QKeyEvent& event = static_cast<const FakeQKeyEvent*>(event_qobj)->event;
    if (is_modifier(event.key()))
        return {};

    const int keyseq_key = event.key() | keymods_to_int(event.modifiers());
    return QKeySequence(keyseq_key);
}
} // namespace utils
