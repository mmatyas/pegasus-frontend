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

#include "model/internal/settings/Themes.h"


class test_Themes : public QObject {
    Q_OBJECT

private:
    int initial_index { -1 };

private slots:
    void initTestCase();

    void indexChange();
    void indexChange_data();
};

void test_Themes::initTestCase()
{
    QTest::ignoreMessage(QtInfoMsg, QRegularExpression("Found theme .*"));
    QTest::ignoreMessage(QtInfoMsg, QRegularExpression("Theme set to .*"));

    model::Themes themes;
    initial_index = themes.currentIndex();

    QVERIFY(initial_index >= 0);
}

void test_Themes::indexChange()
{
    QFETCH(int, testval);

    QTest::ignoreMessage(QtInfoMsg, QRegularExpression("Found theme .*"));
    QTest::ignoreMessage(QtInfoMsg, QRegularExpression("Theme set to .*"));
    if (testval != initial_index)
        QTest::ignoreMessage(QtWarningMsg, QRegularExpression("Invalid theme index .*"));

    model::Themes themes;
    QSignalSpy triggered(&themes, &model::Themes::themeChanged);
    QVERIFY(triggered.isValid());

    const auto before = themes.currentQmlPath();
    themes.setProperty("currentIndex", testval);

    QCOMPARE(themes.property("currentIndex").toInt(), initial_index);
    QCOMPARE(themes.property("currentQmlPath").toString(), before);
    QCOMPARE(triggered.count(), 0);
}

void test_Themes::indexChange_data()
{
    QTest::addColumn<int>("testval");

    QTest::newRow("same") << initial_index;
    QTest::newRow("undefined (-1)") << -1; // there's always at least one theme
    QTest::newRow("out of range (pos)") << 999;
    QTest::newRow("out of range (neg)") << -999;
}


QTEST_MAIN(test_Themes)
#include "test_Themes.moc"
