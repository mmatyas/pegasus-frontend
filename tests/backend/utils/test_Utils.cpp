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

#include "Utils.h"


class test_Utils : public QObject
{
    Q_OBJECT

private slots:
    void validPath_data();
    void validPath();

    void homePath();

    void mathMod();
};

void test_Utils::validPath_data()
{
    QTest::addColumn<QString>("path");
    QTest::addColumn<bool>("result");

    QTest::newRow("null path") << QString() << false;
    QTest::newRow("empty path") << "" << false;
    QTest::newRow("app path") << QCoreApplication::applicationFilePath() << true;
    QTest::newRow("app dir path") << QCoreApplication::applicationDirPath() << true;
    QTest::newRow("not existing path") << "nonexistent" << false;
}

void test_Utils::validPath()
{
    QFETCH(QString, path);
    QFETCH(bool, result);

    QCOMPARE(::validPath(path), result);
}

void test_Utils::homePath()
{
    const QString HOME_ENV_KEY = QStringLiteral("PEGASUS_HOME");
    auto env = QProcessEnvironment::systemEnvironment();

    if (!env.contains(HOME_ENV_KEY))
        QWARN("Set the env var `PEGASUS_HOME` to test this function");
    else
        QCOMPARE(::homePath(), env.value(HOME_ENV_KEY));
}

void test_Utils::mathMod()
{
    int expected = 0;
    for (int i = -6; i <= 6; i++) {
        QCOMPARE(::mathMod(i, 3), expected);
        expected = (expected + 1) % 3;
    }
}


QTEST_MAIN(test_Utils)
#include "test_Utils.moc"
