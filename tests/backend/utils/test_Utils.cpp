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
    void loadInt_data();
    void loadInt();

    void loadFloat_data();
    void loadFloat();

    void validPath_data();
    void validPath();

    void homePath();

    void mathMod();
};

void test_Utils::loadInt_data()
{
    QTest::addColumn<QString>("string");
    QTest::addColumn<int>("value");

    // the conversion itself it already tested in Qt,
    // the point here is the storage behaviour

    QTest::newRow("empty") << "" << 0;
    QTest::newRow("positive number") << "12345" << 12345;
    QTest::newRow("negative number") << "-12345" << -12345;
    QTest::newRow("text") << "abc" << 0;
    QTest::newRow("text with numbers") << "abc123def" << 0;
    QTest::newRow("float") << "1.23" << 0;
    QTest::newRow("max") << "2147483647" << 2147483647;
    QTest::newRow("min") << "-2147483647" << -2147483647;
    QTest::newRow("over max") << "2147483648" << 0;
    QTest::newRow("under min") << "-2147483649" << 0;
}

void test_Utils::loadInt()
{
    int data = 0;

    QFETCH(QString, string);
    QFETCH(int, value);

    parseStoreInt(string, data);

    QCOMPARE(data, value);
}

void test_Utils::loadFloat_data()
{
    QTest::addColumn<QString>("string");
    QTest::addColumn<float>("value");

    // similarly as for int

    QTest::newRow("empty") << "" << 0.f;
    QTest::newRow("positive integer") << "12345" << 12345.f;
    QTest::newRow("negative integer") << "-12345" << -12345.f;
    QTest::newRow("positive float") << "123.45" << 123.45f;
    QTest::newRow("negative float") << "-123.45" << -123.45f;
    QTest::newRow("text") << "abc" << 0.f;
    QTest::newRow("text with numbers") << "abc123def" << 0.f;
}

void test_Utils::loadFloat()
{
    float data = 0;

    QFETCH(QString, string);
    QFETCH(float, value);

    parseStoreFloat(string, data);

    QCOMPARE(data, value);
}

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
