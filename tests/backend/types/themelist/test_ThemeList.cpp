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

#include "types/settings/ThemeList.h"


class test_ThemeList : public QObject {
    Q_OBJECT

private:
    int initial_index;

private slots:
    void initTestCase();

    void indexChange();
    void indexChange_data();

    void count();
};

void test_ThemeList::initTestCase()
{
    QTest::ignoreMessage(QtInfoMsg, QRegularExpression("Found theme .*"));
    QTest::ignoreMessage(QtInfoMsg, QRegularExpression("Theme set to .*"));

    Types::ThemeList themelist;
    QVERIFY(themelist.index() >= 0);

    initial_index = themelist.index();
}

void test_ThemeList::indexChange()
{
    QFETCH(int, testval);

    QTest::ignoreMessage(QtInfoMsg, QRegularExpression("Found theme .*"));
    QTest::ignoreMessage(QtInfoMsg, QRegularExpression("Theme set to .*"));
    if (testval != initial_index)
        QTest::ignoreMessage(QtWarningMsg, QRegularExpression("Invalid theme index .*"));

    Types::ThemeList themelist;
    QSignalSpy triggered(&themelist, &Types::ThemeList::themeChanged);
    QVERIFY(triggered.isValid());

    Types::Theme* before = themelist.current();
    themelist.setProperty("index", testval);

    QCOMPARE(themelist.property("index").toInt(), initial_index);
    QCOMPARE(themelist.property("current").value<Types::Theme*>(), before);
    QCOMPARE(triggered.count(), 0);
}

void test_ThemeList::indexChange_data()
{
    QTest::addColumn<int>("testval");

    QTest::newRow("same") << initial_index;
    QTest::newRow("undefined (-1)") << -1; // there's always at least one theme
    QTest::newRow("out of range (pos)") << 999;
    QTest::newRow("out of range (neg)") << -999;
}

void test_ThemeList::count()
{
    QTest::ignoreMessage(QtInfoMsg, QRegularExpression("Found theme .*"));
    QTest::ignoreMessage(QtInfoMsg, QRegularExpression("Theme set to .*"));

    Types::ThemeList themelist;
    QCOMPARE(themelist.count(), themelist.property("count").toInt());
}


QTEST_MAIN(test_ThemeList)
#include "test_ThemeList.moc"
