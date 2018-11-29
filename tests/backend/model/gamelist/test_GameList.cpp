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

#include "model/gaming/GameList.h"
#include "modeldata/gaming/CollectionData.h"


class test_GameList : public QObject {
    Q_OBJECT

private slots:
    void empty();
    void nonempty();

    void sortGames();

    void indexChange();
    void indexChange_data();

    void indexIncDecEmpty();
    void indexIncDec();
    void indexIncDec_data();

    void letterJump();

private:
    const model::Game* const null_game = static_cast<model::Game*>(nullptr); // Qt 5.7
};

void test_GameList::empty()
{
    model::GameList list;
    list.setModelData({});

    QCOMPARE(list.property("current").value<model::Game*>(), null_game);
    QCOMPARE(list.property("index").toInt(), -1);
    QCOMPARE(list.property("count").toInt(), 0);
    QCOMPARE(list.property("countAll").toInt(), 0);
}

void test_GameList::nonempty()
{
    QVector<model::Game*> games = {
        new model::Game(modeldata::Game(QFileInfo("a")), this),
        new model::Game(modeldata::Game(QFileInfo("b")), this),
        new model::Game(modeldata::Game(QFileInfo("c")), this),
    };
    model::GameList list;
    list.setModelData(std::move(games));

    QCOMPARE(list.property("current").value<model::Game*>(), list.allGames().first());
    QCOMPARE(list.property("index").toInt(), 0);
    QCOMPARE(list.property("count").toInt(), 3);
    QCOMPARE(list.property("countAll").toInt(), 3);
}

void test_GameList::sortGames()
{
    QVector<model::Game*> games = {
        new model::Game(modeldata::Game(QFileInfo("aaa 3")), this),
        new model::Game(modeldata::Game(QFileInfo("bbb")), this),
        new model::Game(modeldata::Game(QFileInfo("aaa")), this),
    };
    model::GameList list;
    list.setModelData(std::move(games));

    QCOMPARE(list.allGames().at(0)->property("title").toString(), QLatin1String("aaa"));
    QCOMPARE(list.allGames().at(1)->property("title").toString(), QLatin1String("aaa 3"));
    QCOMPARE(list.allGames().at(2)->property("title").toString(), QLatin1String("bbb"));
}

void test_GameList::indexChange()
{
    QVector<model::Game*> games = {
        new model::Game(modeldata::Game(QFileInfo("a")), this),
        new model::Game(modeldata::Game(QFileInfo("b")), this),
    };
    model::GameList list;
    list.setModelData(std::move(games));

    QSignalSpy triggered(&list, &model::GameList::currentChanged);
    QVERIFY(triggered.isValid());
    QVERIFY(triggered.count() == 0);
    QVERIFY(list.property("index").toInt() == 0);


    QFETCH(int, target);
    QFETCH(int, expected);
    if (target != expected)
        QTest::ignoreMessage(QtWarningMsg, QRegularExpression("Invalid game index.*"));

    list.setProperty("index", target);

    QCOMPARE(list.property("index").toInt(), expected);
    QCOMPARE(triggered.count(), expected == 0 ? 0 : 1);
    model::Game* current_ptr = list.property("current").value<model::Game*>();
    if (expected == -1) {
        QCOMPARE(current_ptr, null_game);
    }
    else {
        Q_ASSERT(0 <= expected && expected < list.allGames().count());
        QCOMPARE(current_ptr, list.allGames().at(expected));
    }
}

void test_GameList::indexChange_data()
{
    QTest::addColumn<int>("target");
    QTest::addColumn<int>("expected");

    QTest::newRow("same") << 0 << 0;
    QTest::newRow("different") << 1 << 1;
    QTest::newRow("undefined (-1)") << -1 << -1;
    QTest::newRow("out of range (pos)") << 999 << 0;
    QTest::newRow("out of range (neg)") << -999 << 0;
}

void test_GameList::indexIncDecEmpty()
{
    model::GameList list;
    list.setModelData({});
    QVERIFY(list.property("index").toInt() == -1);

    // increment empty -> stays -1
    QMetaObject::invokeMethod(&list, "incrementIndex", Qt::DirectConnection);;
    QCOMPARE(list.property("index").toInt(), -1);

    // decrement empty -> stays -1
    QMetaObject::invokeMethod(&list, "decrementIndex", Qt::DirectConnection);
    QCOMPARE(list.property("index").toInt(), -1);
}

void test_GameList::indexIncDec_data()
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

void test_GameList::indexIncDec()
{
    QVector<model::Game*> games = {
        new model::Game(modeldata::Game(QFileInfo("aaa")), this),
        new model::Game(modeldata::Game(QFileInfo("bbb")), this),
    };
    model::GameList list;
    list.setModelData(std::move(games));

    QFETCH(int, start_idx);
    QFETCH(QString, metacall);
    QFETCH(int, expected_idx);

    list.setProperty("index", start_idx);
    QVERIFY(list.property("index").toInt() == start_idx);

    QMetaObject::invokeMethod(&list, metacall.toStdString().c_str(), Qt::DirectConnection);
    QCOMPARE(list.property("index").toInt(), expected_idx);
}

void test_GameList::letterJump()
{
    QVector<model::Game*> games = {
        new model::Game(modeldata::Game(QFileInfo("Alpha")), this),
        new model::Game(modeldata::Game(QFileInfo("Beta")), this),
        new model::Game(modeldata::Game(QFileInfo("Gamma")), this),
    };
    model::GameList list;
    list.setModelData(std::move(games));
    QVERIFY(list.property("index").toInt() == 0);

    QMetaObject::invokeMethod(&list, "jumpToLetter", Q_ARG(QString, "g"));
    QVERIFY(list.property("index").toInt() == 2);

    QMetaObject::invokeMethod(&list, "jumpToLetter", Q_ARG(QString, "b"));
    QVERIFY(list.property("index").toInt() == 1);
}


QTEST_MAIN(test_GameList)
#include "test_GameList.moc"
