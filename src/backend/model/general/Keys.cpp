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

#include "model/Key.h"


namespace model {

Keys::Keys(QObject* parent)
    : QObject(parent)
{}

// NOTE: Don't call this for invalid and internal KeyEvents.
QVariantList Keys::qmlkeys_of(KeyEvent event) const
{
    QVariantList key_list;

    const auto keyseq_list = AppSettings::keys.at(event);
    for (const QKeySequence& keyseq : keyseq_list)
        key_list << QVariant::fromValue(model::Key(keyseq));

    return key_list;
}

} // namespace model
