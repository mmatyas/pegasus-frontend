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


#include "Keys.h"

#include "AppSettings.h"
#include "model/Key.h"
#include "model/ListPropertyFn.h"


namespace {
void free_keylist(QVector<model::Key*>& keylist)
{
    for (model::Key* keyptr : keylist)
        keyptr->deleteLater();

    keylist.clear();
}
} // namespace


namespace model {

Keys::Keys(QObject* parent)
    : QObject(parent)
    , m_keylists {
        { KeyEvent::LEFT, {} },
        { KeyEvent::RIGHT, {} },
        { KeyEvent::UP, {} },
        { KeyEvent::DOWN, {} },
        { KeyEvent::ACCEPT, {} },
        { KeyEvent::CANCEL, {} },
        { KeyEvent::DETAILS, {} },
        { KeyEvent::FILTERS, {} },
        { KeyEvent::NEXT_PAGE, {} },
        { KeyEvent::PREV_PAGE, {} },
        { KeyEvent::PAGE_UP, {} },
        { KeyEvent::PAGE_DOWN, {} },
        { KeyEvent::MAIN_MENU, {} },
    }
{
    // NOTE: m_keylists is initialized with all keys
    // in order to keep the vector pointers constant
    refresh_keys();
}

void Keys::refresh_keys()
{
    for (auto& entry : m_keylists) {
        auto& keylist = entry.second;
        free_keylist(keylist);

        const auto& keyseq_list = AppSettings::keys.at(entry.first);
        keylist.reserve(keyseq_list.size());

        for (const QKeySequence& keyseq : keyseq_list)
            keylist.append(new model::Key(keyseq, this));

        keylist.squeeze();
    }

    emit keysChanged();
}

bool Keys::qmlkey_in_keylist(KeyEvent keytype, const QVariant& qmlevent) const
{
    const QKeySequence keyseq = ::qmlevent_to_keyseq(qmlevent);
    return AppSettings::keys.at(keytype).count(keyseq);
}

QQmlListProperty<model::Key> Keys::to_qmlkeys(KeyEvent keytype)
{
    static constexpr auto count = &listproperty_count<model::Key>;
    static constexpr auto at = &listproperty_at<model::Key>;

    return {this, &m_keylists.at(keytype), count, at};
}


} // namespace model
