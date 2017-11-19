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

#include "types/PlatformList.h"


class test_PlatformList : public QObject {
    Q_OBJECT

private slots:
    void empty();

    void addPlatform();

    void indexChange();
    void indexChange_data();
};

void test_PlatformList::empty()
{
    Types::PlatformList list;

    QCOMPARE(list.property("current").value<Types::Platform*>(), nullptr);
    QCOMPARE(list.property("index").toInt(), -1);
    QCOMPARE(list.property("count").toInt(), 0);
}

void test_PlatformList::addPlatform()
{
    Types::PlatformList list;
    QSignalSpy spy_platform(&list, &Types::PlatformList::currentChanged);
    QSignalSpy spy_game(&list, &Types::PlatformList::currentPlatformGameChanged);
    QVERIFY(spy_platform.isValid());
    QVERIFY(spy_game.isValid());
    QTest::ignoreMessage(QtInfoMsg, QRegularExpression("\\d+ games found"));

    // TODO: implement addPlatform
    list.platformsMut().append(new Types::Platform());
    list.platformsMut().last()->gameListMut().addGame("dummy1");
    list.platformsMut().append(new Types::Platform());
    list.platformsMut().last()->gameListMut().addGame("dummy2");
    list.onScanComplete();

    QCOMPARE(list.property("current").value<Types::Platform*>(), list.platforms().first());
    QCOMPARE(list.property("index").toInt(), 0);
    QCOMPARE(list.property("count").toInt(), 2);
    QCOMPARE(spy_platform.count(), 1);
    QCOMPARE(spy_game.count(), 1);
}

void test_PlatformList::indexChange()
{
    // prepare

    Types::PlatformList list;
    QSignalSpy spy_platform(&list, &Types::PlatformList::currentChanged);
    QSignalSpy spy_game(&list, &Types::PlatformList::currentPlatformGameChanged);
    QVERIFY(spy_platform.isValid());
    QVERIFY(spy_game.isValid());
    QTest::ignoreMessage(QtInfoMsg, QRegularExpression("\\d+ games found"));

    // TODO: implement addPlatform
    list.platformsMut().append(new Types::Platform());
    list.platformsMut().last()->gameListMut().addGame("dummy1");
    list.platformsMut().append(new Types::Platform());
    list.platformsMut().last()->gameListMut().addGame("dummy2");
    list.onScanComplete();

    QVERIFY(list.count() == 2);
    QVERIFY(list.index() == 0);
    QVERIFY(spy_platform.count() == 1);
    QVERIFY(spy_game.count() == 1);

    // test

    QFETCH(int, target);
    QFETCH(int, expected);
    if (target != expected)
        QTest::ignoreMessage(QtWarningMsg, QRegularExpression("Invalid platform index.*"));

    list.setProperty("index", target);

    QCOMPARE(list.property("index").toInt(), expected);
    QCOMPARE(spy_platform.count(), expected == 0 ? 1 : 2);
    QCOMPARE(spy_game.count(), expected == 0 ? 1 : 2);

    // check pointer
    auto current_ptr = list.property("current").value<Types::Platform*>();
    if (expected == -1) {
        QCOMPARE(current_ptr, nullptr);
    }
    else {
        Q_ASSERT(0 <= expected && expected < list.platforms().count());
        QCOMPARE(current_ptr, list.platforms().at(expected));
    }
}

void test_PlatformList::indexChange_data()
{
    QTest::addColumn<int>("target");
    QTest::addColumn<int>("expected");

    QTest::newRow("same") << 0 << 0;
    QTest::newRow("different") << 1 << 1;
    QTest::newRow("undefined (-1)") << -1 << -1;
    QTest::newRow("out of range (pos)") << 999 << 0;
    QTest::newRow("out of range (neg)") << -999 << 0;
}


QTEST_MAIN(test_PlatformList)
#include "test_PlatformList.moc"
