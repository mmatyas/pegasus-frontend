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


#include <QtQuickTest>

#include "model/keys/Key.h"
#include "model/keys/Keys.h"
#include "model/internal/settings/KeyEditor.h"

#include <QQmlEngine>
#include <QQmlContext>


class Setup : public QObject {
    Q_OBJECT

public:
    Setup()
#ifdef Q_OS_MAC
        : m_is_mac(true)
#else
        : m_is_mac(false)
#endif
    {
        qmlRegisterUncreatableType<model::Key>("PegasusTest", 1, 0, "Key", "Error!");

        connect(&m_keyeditor, &model::KeyEditor::keysChanged,
                &m_keys, &model::Keys::refresh_keys);
    }

public slots:
    void qmlEngineAvailable(QQmlEngine *engine)
    {
        auto ctx = engine->rootContext();
        ctx->setContextProperty("keys", &m_keys);
        ctx->setContextProperty("keyEditor", &m_keyeditor);
        ctx->setContextProperty("isMac", QVariant::fromValue(m_is_mac));
    }

private:
    const bool m_is_mac;
    model::KeyEditor m_keyeditor;
    model::Keys m_keys;
};


QUICK_TEST_MAIN_WITH_SETUP(KeyEditor, Setup)
#include "test_KeyEditor.moc"
