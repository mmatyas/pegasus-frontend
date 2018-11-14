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


#pragma once

#include "AppSettings.h"
#include "utils/KeySequenceTools.h"

#include <QObject>
#include <QVector>
#include <QKeySequence>


namespace model {

class Keys : public QObject {
    Q_OBJECT

public:
    #define KEYVEC_PROP(keytype, qml_fn) \
        Q_INVOKABLE bool qml_fn(const QVariant& qmlevent) const { \
            return AppSettings::keys.at(KeyEvent::keytype).count(::qmlevent_to_keyseq(qmlevent)); \
        }
    KEYVEC_PROP(LEFT, isLeft)
    KEYVEC_PROP(RIGHT, isRight)
    KEYVEC_PROP(UP, isUp)
    KEYVEC_PROP(DOWN, isDown)
    KEYVEC_PROP(ACCEPT, isAccept)
    KEYVEC_PROP(CANCEL, isCancel)
    KEYVEC_PROP(DETAILS, isDetails)
    KEYVEC_PROP(FILTERS, isFilters)
    KEYVEC_PROP(NEXT_PAGE, isNextPage)
    KEYVEC_PROP(PREV_PAGE, isPrevPage)
    KEYVEC_PROP(PAGE_UP, isPageUp)
    KEYVEC_PROP(PAGE_DOWN, isPageDown)
    KEYVEC_PROP(MAIN_MENU, isMenu)
    #undef KEYVEC_PROP

public:
    explicit Keys(QObject* parent = nullptr);

signals:
    void keysChanged();
};

} // namespace model


