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

#include "CustomFilters.h"


class test_Filters : public QObject {
    Q_OBJECT

private slots:
    void read_empty();
};

void test_Filters::read_empty()
{
    QTest::ignoreMessage(QtInfoMsg, QRegularExpression("Found \\d+ custom filters"));

    QTemporaryFile tmp_file;
    tmp_file.open();

    const auto result = CustomFilters::read(tmp_file.fileName());
    QCOMPARE(result.count(), 0);
}


QTEST_MAIN(test_Filters)
#include "test_Filters.moc"
