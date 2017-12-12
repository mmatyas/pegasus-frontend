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

#include "types/LocaleList.h"


class test_LocaleList : public QObject {
    Q_OBJECT

private:
    int initial_index;

private slots:
    void initTestCase();

    void indexChange();
    void indexChange_data();

    void count();
};

void test_LocaleList::initTestCase()
{
    QTest::ignoreMessage(QtInfoMsg, QRegularExpression("Found locale .*"));
    QTest::ignoreMessage(QtInfoMsg, QRegularExpression("Locale set to .*"));

    Types::LocaleList localelist;
    QVERIFY(localelist.index() >= 0);

    initial_index = localelist.index();
}

void test_LocaleList::indexChange()
{
    QFETCH(int, testval);

    QTest::ignoreMessage(QtInfoMsg, QRegularExpression("Found locale .*"));
    QTest::ignoreMessage(QtInfoMsg, QRegularExpression("Locale set to .*"));
    if (testval != initial_index)
        QTest::ignoreMessage(QtWarningMsg, QRegularExpression("Invalid locale index .*"));

    Types::LocaleList localelist;
    QSignalSpy triggered(&localelist, &Types::LocaleList::localeChanged);
    QVERIFY(triggered.isValid());

    Types::Locale* before = localelist.current();
    localelist.setProperty("index", testval);

    QCOMPARE(localelist.property("index").toInt(), initial_index);
    QCOMPARE(localelist.property("current").value<Types::Locale*>(), before);
    QCOMPARE(triggered.count(), 0);
}

void test_LocaleList::indexChange_data()
{
    QTest::addColumn<int>("testval");

    QTest::newRow("same") << initial_index;
    QTest::newRow("undefined (-1)") << -1; // there's always at least one locale
    QTest::newRow("out of range (pos)") << 999;
    QTest::newRow("out of range (neg)") << -999;
}

void test_LocaleList::count()
{
    QTest::ignoreMessage(QtInfoMsg, QRegularExpression("Found locale .*"));
    QTest::ignoreMessage(QtInfoMsg, QRegularExpression("Locale set to .*"));

    Types::LocaleList localelist;
    QCOMPARE(localelist.count(), localelist.property("count").toInt());
}


QTEST_MAIN(test_LocaleList)
#include "test_LocaleList.moc"
