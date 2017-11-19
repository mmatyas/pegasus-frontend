// Pegasus Frontend
// Copyright (C) 2017  Mátyás Mustoha
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


#include <QtTest/QtTest>

#include "types/Platform.h"


class test_Platform : public QObject {
    Q_OBJECT

private slots:
    void names();
    void gameChanged();
};

void test_Platform::names()
{
    Types::Platform platform;
    platform.setShortName("short");
    platform.setLongName("long");
    platform.setCommonLaunchCmd("runner");

    // the properties are read-only and should be called only after the initial setup
    QCOMPARE(platform.property("shortName").toString(), QStringLiteral("short"));
    QCOMPARE(platform.property("longName").toString(), QStringLiteral("long"));
}

void test_Platform::gameChanged()
{
    Types::Platform platform;
    QSignalSpy triggered(&platform, &Types::Platform::currentGameChanged);
    QVERIFY(triggered.isValid());

    platform.gameListMut().addGame("dummy1");
    platform.gameListMut().addGame("dummy2");

    platform.gameListMut().lockGameList();
    QCOMPARE(triggered.count(), 1);

    platform.gameListMut().setIndex(1);
    QCOMPARE(triggered.count(), 2);
}


QTEST_MAIN(test_Platform)
#include "test_Platform.moc"
