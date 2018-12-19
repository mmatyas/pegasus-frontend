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

#include "types/KeyEventType.h"
#include "utils/HashMap.h"

#include <QList>
#include <QObject>
#include <QVector>


namespace model {

class Keys : public QObject {
    Q_OBJECT

    #define KEYVEC_PROP(keytype, keylist, checkFn) \
        private: \
            Q_PROPERTY(QList<QObject*> keylist READ keylist NOTIFY keysChanged) \
            QList<QObject*> keylist() { \
                return to_qmlkeys(KeyEvent::keytype); \
            } \
        public: \
            Q_INVOKABLE bool checkFn(const QVariant& qmlEvent) const { \
                return qmlkey_in_keylist(KeyEvent::keytype, qmlEvent); \
            }
    KEYVEC_PROP(LEFT, left, isLeft)
    KEYVEC_PROP(RIGHT, right, isRight)
    KEYVEC_PROP(UP, up, isUp)
    KEYVEC_PROP(DOWN, down, isDown)
    KEYVEC_PROP(ACCEPT, accept, isAccept)
    KEYVEC_PROP(CANCEL, cancel, isCancel)
    KEYVEC_PROP(DETAILS, details, isDetails)
    KEYVEC_PROP(FILTERS, filters, isFilters)
    KEYVEC_PROP(NEXT_PAGE, nextPage, isNextPage)
    KEYVEC_PROP(PREV_PAGE, prevPage, isPrevPage)
    KEYVEC_PROP(PAGE_UP, pageUp, isPageUp)
    KEYVEC_PROP(PAGE_DOWN, pageDown, isPageDown)
    KEYVEC_PROP(MAIN_MENU, menu, isMenu)
    #undef KEYVEC_PROP

public:
    explicit Keys(QObject* parent = nullptr);

    void refresh_keys();

signals:
    void keysChanged();

private:
    HashMap<KeyEvent, QVector<QObject*>, EnumHash> m_keylists;

    bool qmlkey_in_keylist(KeyEvent, const QVariant&) const;
    QList<QObject*> to_qmlkeys(KeyEvent);
};

} // namespace model


