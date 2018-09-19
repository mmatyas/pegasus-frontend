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

#include <QObject>
#include <QVector>


namespace model {

class Keys : public QObject {
    Q_OBJECT

public:
    #define KEYVEC_PROP(event, qml_array, qml_fn) \
        Q_PROPERTY(QVector<int> qml_array READ qml_array NOTIFY keysChanged) \
        QVector<int> qml_array() const { \
            QVector<int> keys(AppSettings::keys.at(KeyEvent::event)); \
            return keys << AppSettings::keys.gamepadKey(KeyEvent::event); \
        } \
        Q_INVOKABLE bool qml_fn(int key) const { \
            return qml_array().count(key) || AppSettings::keys.gamepadKey(KeyEvent::event) == key; \
        }
    KEYVEC_PROP(ACCEPT, accept, isAccept)
    KEYVEC_PROP(CANCEL, cancel, isCancel)
    KEYVEC_PROP(DETAILS, details, isDetails)
    KEYVEC_PROP(FILTERS, filters, isFilters)
    KEYVEC_PROP(NEXT_PAGE, nextPage, isNextPage)
    KEYVEC_PROP(PREV_PAGE, prevPage, isPrevPage)
    KEYVEC_PROP(PAGE_UP, pageUp, isPageUp)
    KEYVEC_PROP(PAGE_DOWN, pageDown, isPageDown)
    #undef KEYVEC_PROP

public:
    explicit Keys(QObject* parent = nullptr);

signals:
    void keysChanged();
};

} // namespace model


