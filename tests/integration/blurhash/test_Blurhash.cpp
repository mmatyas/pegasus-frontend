// Pegasus Frontend
// Copyright (C) 2017-2020  Mátyás Mustoha
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

#include "backend/imggen/BlurhashProvider.h"

#include <QQmlEngine>
#include <QQmlContext>


class Setup : public QObject {
    Q_OBJECT

public:
    Setup() {}

public slots:
    void qmlEngineAvailable(QQmlEngine* engine)
    {
        engine->addImageProvider(QStringLiteral("blurhash"), new BlurhashProvider);
    }
};


QUICK_TEST_MAIN_WITH_SETUP(Blurhash, Setup)
#include "test_Blurhash.moc"
