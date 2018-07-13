// Pegasus Frontend
// Copyright (C) 2018  Mátyás Mustoha
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

#include "model/Filters.h"
#include "configfiles/CustomFilters.h"


class test_Filters : public QObject {
    Q_OBJECT

private slots:
    void read_empty();
    void read();
};

void test_Filters::read_empty()
{
    QTest::ignoreMessage(QtInfoMsg, QRegularExpression("Found 0 custom filters"));

    QTemporaryFile tmp_file;
    tmp_file.open();

    const auto result = CustomFilters::read(tmp_file.fileName());
    QCOMPARE(result.count(), 0);
}

void test_Filters::read()
{
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("line 2: no filter defined yet, entry ignored"));
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("line 7: unrecognized game property `invalidprop`, rule ignored"));
    QTest::ignoreMessage(QtInfoMsg, QRegularExpression("Found 1 custom filters"));

    const auto result = CustomFilters::read(":/filters.txt");
    QCOMPARE(result.count(), 1);

    QCOMPARE(result.first()->property("name").toString(), QStringLiteral("test"));
    QCOMPARE(result.first()->property("enabled").toBool(), false);
    QCOMPARE(result.first()->rules().count(), 3);
    QCOMPARE(result.first()->rules().at(0).game_property, QStringLiteral("favorite"));
    QCOMPARE(result.first()->rules().at(0).type, model::FilterRuleType::IS_TRUE);
    QCOMPARE(result.first()->rules().at(1).game_property, QStringLiteral("title"));
    QCOMPARE(result.first()->rules().at(1).type, model::FilterRuleType::CONTAINS);
    QCOMPARE(result.first()->rules().at(1).regex.pattern(), QStringLiteral("abc"));
    QCOMPARE(result.first()->rules().at(2).game_property, QStringLiteral("releaseYear"));
    QCOMPARE(result.first()->rules().at(2).type, model::FilterRuleType::NOT_EQUALS);
    QCOMPARE(result.first()->rules().at(2).regex.pattern(), QStringLiteral("2000|2001"));
}


QTEST_MAIN(test_Filters)
#include "test_Filters.moc"
