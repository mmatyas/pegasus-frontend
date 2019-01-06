// Pegasus Frontend
// Copyright (C) 2017-2018  Mátyás Mustoha
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

#include "Container.h"


class test_Memory : public QObject {
    Q_OBJECT

private slots:
    void set_new();
    void set_overwrite();
    void unset();

    void invalid();

    void json();
    void json_data();

    void settings_file();
};

void test_Memory::set_new()
{
    Container c;
    QSignalSpy changed(&c, &Container::memoryChanged);
    QVERIFY(changed.isValid());

    QCOMPARE(c.memory()->has("test"), false);
    QCOMPARE(c.memory()->get("test"), QVariant());

    c.memory()->set("test", "myvalue");
    QCOMPARE(changed.count(), 1);

    QCOMPARE(c.memory()->has("test"), true);
    QCOMPARE(c.memory()->get("test"), QVariant("myvalue"));
}

void test_Memory::set_overwrite()
{
    Container c;
    QSignalSpy changed(&c, &Container::memoryChanged);
    QVERIFY(changed.isValid());

    c.memory()->set("test", "myvalue");
    QCOMPARE(changed.count(), 1);
    QCOMPARE(c.memory()->has("test"), true);
    QCOMPARE(c.memory()->get("test"), QVariant("myvalue"));

    c.memory()->set("test", "something else");
    QCOMPARE(changed.count(), 2);

    QCOMPARE(c.memory()->has("test"), true);
    QCOMPARE(c.memory()->get("test"), QVariant("something else"));
}

void test_Memory::unset()
{
    Container c;
    QSignalSpy changed(&c, &Container::memoryChanged);
    QVERIFY(changed.isValid());

    c.memory()->set("test", "myvalue");
    QCOMPARE(changed.count(), 1);
    QCOMPARE(c.memory()->has("test"), true);
    QCOMPARE(c.memory()->get("test"), QVariant("myvalue"));

    c.memory()->unset("test");
    QCOMPARE(changed.count(), 2);
    QCOMPARE(c.memory()->has("test"), false);
    QCOMPARE(c.memory()->get("test"), QVariant());
}

void test_Memory::invalid()
{
    Container c;
    QSignalSpy changed(&c, &Container::memoryChanged);
    QVERIFY(changed.isValid());

    QTest::ignoreMessage(QtWarningMsg, "`set(key,val)` called with empty `key`, ignored");
    QTest::ignoreMessage(QtWarningMsg, "`set(key,val)` called with invalid `val` type, ignored");

    c.memory()->set("", "myvalue");
    QCOMPARE(changed.count(), 0);
    QCOMPARE(c.memory()->has(""), false);
    QCOMPARE(c.memory()->get(""), QVariant());

    c.memory()->set("test", QVariant());
    QCOMPARE(changed.count(), 0);
    QCOMPARE(c.memory()->has("test"), false);
    QCOMPARE(c.memory()->get("test"), QVariant());
}

void test_Memory::json()
{
    Container c;
    QSignalSpy changed(&c, &Container::memoryChanged);
    QVERIFY(changed.isValid());

    QFETCH(QJsonValue, jsonval);

    c.memory()->set("test", QVariant(jsonval));
    QCOMPARE(changed.count(), 1);
    QCOMPARE(c.memory()->has("test"), true);
    QCOMPARE(c.memory()->get("test"), QVariant(jsonval));
}

void test_Memory::json_data()
{
    QTest::addColumn<QJsonValue>("jsonval");
    QTest::newRow("null") << QJsonValue(QJsonValue::Null);
    QTest::newRow("undefined") << QJsonValue(QJsonValue::Undefined);
    QTest::newRow("array") << QJsonValue(QJsonArray({1,2,3}));
    QTest::newRow("object") << QJsonValue(QJsonObject({{"x", 1}, {"y", 2}}));
}

void test_Memory::settings_file()
{
    QString temp_path = QDir::tempPath();
    if (!temp_path.endsWith('/'))
        temp_path += '/';

    const QString json_path_a = temp_path + "QtAutoTestA.json";
    const QString json_path_b = temp_path + "QtAutoTestB.json";
    QFile(json_path_a).remove();
    QFile(json_path_b).remove();


    Container c(temp_path);
    QSignalSpy changed(&c, &Container::memoryChanged);
    QVERIFY(changed.isValid());
    int change_cnt = 0;

    // set some values

    c.memory()->changeTheme("/path/to/QtAutoTestA/");
    QCOMPARE(changed.count(), ++change_cnt);

    c.memory()->set("test", "myvalue");
    QCOMPARE(changed.count(), ++change_cnt);

    // data is cleared on theme change

    c.memory()->changeTheme("/path/to/QtAutoTestB/");
    QCOMPARE(changed.count(), ++change_cnt);
    QCOMPARE(c.memory()->has("test"), false);
    QCOMPARE(c.memory()->get("test"), QVariant());

    // the data before the change was saved

    QCOMPARE(QFileInfo::exists(json_path_a), true);
    QFile json_file(json_path_a);
    json_file.open(QFile::ReadOnly);
    QCOMPARE(json_file.readAll(), QByteArrayLiteral(R"({"test":"myvalue"})"));

    // data is reloaded on changing the theme back

    c.memory()->changeTheme("/path/to/QtAutoTestA/");
    QCOMPARE(changed.count(), ++change_cnt);
    QCOMPARE(c.memory()->has("test"), true);
    QCOMPARE(c.memory()->get("test"), QVariant("myvalue"));

    // empty data is not saved

    QCOMPARE(QFileInfo::exists(json_path_b), false);

    json_file.remove();
}


QTEST_MAIN(test_Memory)
#include "test_Memory.moc"
