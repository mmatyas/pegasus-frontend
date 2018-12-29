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
    void set_delete();

    void invalid();

    void settings_file();
};

void test_Memory::set_new()
{
    Container c;
    QVariant result;
    QSignalSpy changed(&c, &Container::memoryChanged);
    QVERIFY(changed.isValid());

    result = c.memory()->get("test");
    QCOMPARE(result, QVariant());

    c.memory()->set("test", "myvalue");
    QCOMPARE(changed.count(), 1);

    result = c.memory()->get("test");
    QCOMPARE(result, QVariant("myvalue"));
}

void test_Memory::set_overwrite()
{
    Container c;
    QVariant result;
    QSignalSpy changed(&c, &Container::memoryChanged);
    QVERIFY(changed.isValid());

    c.memory()->set("test", "myvalue");
    QCOMPARE(changed.count(), 1);

    result = c.memory()->get("test");
    QCOMPARE(result, QVariant("myvalue"));

    c.memory()->set("test", "something else");
    QCOMPARE(changed.count(), 2);

    result = c.memory()->get("test");
    QCOMPARE(result, QVariant("something else"));
}

void test_Memory::set_delete()
{
    Container c;
    QVariant result;
    QSignalSpy changed(&c, &Container::memoryChanged);
    QVERIFY(changed.isValid());

    c.memory()->set("test", "myvalue");
    QCOMPARE(changed.count(), 1);

    result = c.memory()->get("test");
    QCOMPARE(result, QVariant("myvalue"));

    c.memory()->set("test", QVariant());
    QCOMPARE(changed.count(), 2);

    result = c.memory()->get("test");
    QCOMPARE(result, QVariant());
}

void test_Memory::invalid()
{
    Container c;
    QVariant result;
    QSignalSpy changed(&c, &Container::memoryChanged);
    QVERIFY(changed.isValid());

    QTest::ignoreMessage(QtWarningMsg, "`set(key,val)` called with empty `key`, ignored");

    c.memory()->set("", "myvalue");
    QCOMPARE(changed.count(), 0);

    result = c.memory()->get("");
    QCOMPARE(result, QVariant());
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
    QCOMPARE(c.memory()->get("test"), QVariant());

    // the data before the change was saved

    QCOMPARE(QFileInfo::exists(json_path_a), true);
    QFile json_file(json_path_a);
    json_file.open(QFile::ReadOnly);
    QCOMPARE(json_file.readAll(), QByteArrayLiteral(R"({"test":"myvalue"})"));

    // data is reloaded on changing the theme back

    c.memory()->changeTheme("/path/to/QtAutoTestA/");
    QCOMPARE(changed.count(), ++change_cnt);
    QCOMPARE(c.memory()->get("test"), QVariant("myvalue"));

    // empty data is not saved

    QCOMPARE(QFileInfo::exists(json_path_b), false);

    json_file.remove();
}


QTEST_MAIN(test_Memory)
#include "test_Memory.moc"
