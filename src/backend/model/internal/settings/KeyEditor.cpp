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

#include "AppSettings.h"
#include "utils/KeySequenceTools.h"


namespace {
bool valid_event_id(int event_id)
{
    return 0 <= event_id && event_id < model::KeyEditor::eventCount();
}
} // namespace


namespace model {

KeyEditor::KeyEditor(QObject* parent)
    : QObject(parent)
{}

void KeyEditor::addKey(int event_id, const QVariant& event)
{
    if (!valid_event_id(event_id))
        return;

    QKeySequence keyseq = ::qmlevent_to_keyseq(event);
    if (keyseq.isEmpty())
        return;

    AppSettings::keys.add_key(static_cast<::KeyEvent>(event_id), std::move(keyseq));
    AppSettings::save_config();
    emit keysChanged();
}

void KeyEditor::deleteKeyCode(int event_id, const int keycode)
{
    if (!valid_event_id(event_id) || keycode == 0)
        return;

    AppSettings::keys.del_key(static_cast<::KeyEvent>(event_id), QKeySequence(keycode));
    AppSettings::save_config();
    emit keysChanged();
}

void KeyEditor::replaceKeyCode(int event_id, const int old_keycode, const QVariant& new_keyevent)
{
    if (!valid_event_id(event_id) || old_keycode == 0)
        return;

    QKeySequence keyseq_new = ::qmlevent_to_keyseq(new_keyevent);
    if (keyseq_new.isEmpty())
        return;

    AppSettings::keys.del_key(static_cast<::KeyEvent>(event_id), QKeySequence(old_keycode));
    AppSettings::keys.add_key(static_cast<::KeyEvent>(event_id), std::move(keyseq_new));
    AppSettings::save_config();
    emit keysChanged();
}

void KeyEditor::resetKeys()
{
    AppSettings::keys.resetAll();
    AppSettings::save_config();
    emit keysChanged();
}

} // namespace model
