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

#include "types/gaming/CollectionList.h"


class test_CollectionList : public QObject {
    Q_OBJECT

private slots:
    void empty();

    void addPlatform();

    void indexChange();
    void indexChange_data();

    void indexIncDecEmpty();
    void indexIncDec();
    void indexIncDec_data();

private:
    const types::Collection* const null_coll = static_cast<types::Collection*>(nullptr); // Qt 5.7
};

void test_CollectionList::empty()
{
    types::CollectionList list;

    QCOMPARE(list.property("current").value<types::Collection*>(), null_coll);
    QCOMPARE(list.property("index").toInt(), -1);
    QCOMPARE(list.property("count").toInt(), 0);
}

void test_CollectionList::addPlatform()
{
    types::CollectionList list;
    QSignalSpy spy_current(&list, &types::CollectionList::currentChanged);
    QSignalSpy spy_game(&list, &types::CollectionList::currentGameChanged);
    QVERIFY(spy_current.isValid());
    QVERIFY(spy_game.isValid());
    QTest::ignoreMessage(QtInfoMsg, QRegularExpression("\\d+ games found"));

    // TODO: implement addPlatform
    list.elementsMut().append(new types::Collection("coll1"));
    list.elementsMut().last()->gameListMut().addGame("dummy1");
    list.elementsMut().append(new types::Collection("coll2"));
    list.elementsMut().last()->gameListMut().addGame("dummy2");
    list.onScanComplete();

    QCOMPARE(list.property("current").value<types::Collection*>(), list.elements().first());
    QCOMPARE(list.property("index").toInt(), 0);
    QCOMPARE(list.property("count").toInt(), 2);
    QCOMPARE(spy_current.count(), 1);
    QCOMPARE(spy_game.count(), 1);
}

void test_CollectionList::indexChange()
{
    // prepare

    types::CollectionList list;
    QSignalSpy spy_current(&list, &types::CollectionList::currentChanged);
    QSignalSpy spy_game(&list, &types::CollectionList::currentGameChanged);
    QVERIFY(spy_current.isValid());
    QVERIFY(spy_game.isValid());
    QTest::ignoreMessage(QtInfoMsg, QRegularExpression("\\d+ games found"));

    // TODO: implement addPlatform
    list.elementsMut().append(new types::Collection("coll1"));
    list.elementsMut().last()->gameListMut().addGame("dummy1");
    list.elementsMut().append(new types::Collection("coll2"));
    list.elementsMut().last()->gameListMut().addGame("dummy2");
    list.onScanComplete();

    QVERIFY(list.count() == 2);
    QVERIFY(list.index() == 0);
    QVERIFY(spy_current.count() == 1);
    QVERIFY(spy_game.count() == 1);

    // test

    QFETCH(int, target);
    QFETCH(int, expected);
    if (target != expected)
        QTest::ignoreMessage(QtWarningMsg, QRegularExpression("Invalid collection index.*"));

    list.setProperty("index", target);

    QCOMPARE(list.property("index").toInt(), expected);
    QCOMPARE(spy_current.count(), expected == 0 ? 1 : 2);
    QCOMPARE(spy_game.count(), expected == 0 ? 1 : 2);

    // check pointer
    auto current_ptr = list.property("current").value<types::Collection*>();
    if (expected == -1) {
        QCOMPARE(current_ptr, null_coll);
    }
    else {
        Q_ASSERT(0 <= expected && expected < list.elements().count());
        QCOMPARE(current_ptr, list.elements().at(expected));
    }
}

void test_CollectionList::indexChange_data()
{
    QTest::addColumn<int>("target");
    QTest::addColumn<int>("expected");

    QTest::newRow("same") << 0 << 0;
    QTest::newRow("different") << 1 << 1;
    QTest::newRow("undefined (-1)") << -1 << -1;
    QTest::newRow("out of range (pos)") << 999 << 0;
    QTest::newRow("out of range (neg)") << -999 << 0;
}

void test_CollectionList::indexIncDecEmpty()
{
    types::CollectionList list;
    QVERIFY(list.index() == -1);

    // increment empty -> stays -1
    QMetaObject::invokeMethod(&list, "incrementIndex", Qt::DirectConnection);
    QCOMPARE(list.index(), -1);

    // decrement empty -> stays -1
    QMetaObject::invokeMethod(&list, "decrementIndex", Qt::DirectConnection);
    QCOMPARE(list.index(), -1);
}

void test_CollectionList::indexIncDec_data()
{
    QTest::addColumn<int>("start_idx");
    QTest::addColumn<QString>("metacall");
    QTest::addColumn<int>("expected_idx");

    QTest::newRow("increment regular") << 0 << "incrementIndex" << 1;
    QTest::newRow("increment last") << 1 << "incrementIndex" << 0;
    QTest::newRow("decrement regular") << 1 << "decrementIndex" << 0;
    QTest::newRow("decrement first") << 0 << "decrementIndex" << 1;
    QTest::newRow("increment regular, no wrap") << 0 << "incrementIndexNoWrap" << 1;
    QTest::newRow("increment last, no wrap") << 1 << "incrementIndexNoWrap" << 1;
    QTest::newRow("decrement regular, no wrap") << 1 << "decrementIndexNoWrap" << 0;
    QTest::newRow("decrement first, no wrap") << 0 << "decrementIndexNoWrap" << 0;
}

void test_CollectionList::indexIncDec()
{
    types::CollectionList list;
    list.elementsMut().append(new types::Collection("coll1"));
    list.elementsMut().last()->gameListMut().addGame("dummy1");
    list.elementsMut().append(new types::Collection("coll2"));
    list.elementsMut().last()->gameListMut().addGame("dummy1");
    QTest::ignoreMessage(QtInfoMsg, QRegularExpression("\\d+ games found"));
    list.onScanComplete();

    QFETCH(int, start_idx);
    QFETCH(QString, metacall);
    QFETCH(int, expected_idx);

    list.setIndex(start_idx);
    QVERIFY(list.index() == start_idx);

    QMetaObject::invokeMethod(&list, metacall.toStdString().c_str(), Qt::DirectConnection);
    QCOMPARE(list.index(), expected_idx);
}


QTEST_MAIN(test_CollectionList)
#include "test_CollectionList.moc"
