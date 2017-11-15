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

private slots:
    void neverEmpty();

    void indexChange();
    void indexChange_data();
};

void test_LocaleList::neverEmpty()
{
    QTest::ignoreMessage(QtInfoMsg, QRegularExpression("Found locale.*"));
    QTest::ignoreMessage(QtInfoMsg, QRegularExpression("Locale set to .*"));

    Types::LocaleList localelist;
    QVERIFY(localelist.index() >= 0);
}

void test_LocaleList::indexChange()
{
    QTest::ignoreMessage(QtInfoMsg, QRegularExpression("Found locale.*"));
    QTest::ignoreMessage(QtInfoMsg, QRegularExpression("Locale set to .*"));
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("Invalid locale index .*"));

    Types::LocaleList localelist;
    int initial_index = localelist.index();

    QFETCH(int, testval);

    localelist.setIndex(testval);
    QCOMPARE(localelist.index(), initial_index);
}

void test_LocaleList::indexChange_data()
{
    QTest::addColumn<int>("testval");

    QTest::newRow("undefined (-1)") << -1;
    QTest::newRow("out of range (pos)") << 999;
    QTest::newRow("out of range (neg)") << -999;
}


QTEST_MAIN(test_LocaleList)
#include "test_LocaleList.moc"
