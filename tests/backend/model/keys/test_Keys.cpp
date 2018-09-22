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

#include "AppSettings.h"
#include "model/settings/KeyEditor.h"


class test_Keys : public QObject {
    Q_OBJECT

private slots:
    void init();
    void addNew();
    void addExisting();
    void addExisting2();
    void del();
    void change();
    void change2();
    void keyList();
    void keyName();
};

void test_Keys::init()
{
    AppSettings::general.portable = true;
    AppSettings::keys.resetAll();
}

void test_Keys::addNew()
{
    model::KeyEditor keyeditor;

    QSignalSpy triggered(&keyeditor, &model::KeyEditor::keysChanged);
    QVERIFY(triggered.isValid());

    constexpr auto event = KeyEvent::ACCEPT;
    constexpr auto new_key = Qt::Key_X;
    QVERIFY(AppSettings::keys.at(event).count(new_key) == 0);

    QTest::ignoreMessage(QtInfoMsg, "Program settings saved");
    keyeditor.addKey(static_cast<int>(event), new_key);

    QCOMPARE(triggered.count(), 1);
    QVERIFY(AppSettings::keys.at(event).count(new_key) == 1);
}

void test_Keys::addExisting()
{
    model::KeyEditor keyeditor;

    constexpr auto event = KeyEvent::ACCEPT;
    const auto new_key = AppSettings::keys.at(event).constFirst();
    QVERIFY(AppSettings::keys.at(event).count(new_key) == 1);

    QTest::ignoreMessage(QtInfoMsg, "Program settings saved");
    keyeditor.addKey(static_cast<int>(event), new_key);

    QVERIFY(AppSettings::keys.at(event).count(new_key) == 1);
}

void test_Keys::addExisting2()
{
    model::KeyEditor keyeditor;

    QSignalSpy triggered(&keyeditor, &model::KeyEditor::keysChanged);
    QVERIFY(triggered.isValid());

    constexpr auto event1 = KeyEvent::ACCEPT;
    constexpr auto event2 = KeyEvent::CANCEL;
    const auto key = AppSettings::keys.at(event2).constFirst();
    QVERIFY(AppSettings::keys.at(event1).count(key) == 0);
    QVERIFY(AppSettings::keys.at(event2).count(key) == 1);

    QTest::ignoreMessage(QtInfoMsg, "Program settings saved");
    keyeditor.addKey(static_cast<int>(event1), key);

    QCOMPARE(triggered.count(), 1);
    QVERIFY(AppSettings::keys.at(event1).count(key) == 1);
    QVERIFY(AppSettings::keys.at(event2).count(key) == 0);
}

void test_Keys::del()
{
    model::KeyEditor keyeditor;

    QSignalSpy triggered(&keyeditor, &model::KeyEditor::keysChanged);
    QVERIFY(triggered.isValid());

    constexpr auto event = KeyEvent::ACCEPT;
    const auto deleted_key = AppSettings::keys.at(event).constFirst();
    QVERIFY(AppSettings::keys.at(event).count(deleted_key) == 1);

    QTest::ignoreMessage(QtInfoMsg, "Program settings saved");
    keyeditor.delKey(static_cast<int>(event), deleted_key);

    QCOMPARE(triggered.count(), 1);
    QVERIFY(AppSettings::keys.at(event).count(deleted_key) == 0);
}

void test_Keys::change()
{
    model::KeyEditor keyeditor;

    QSignalSpy triggered(&keyeditor, &model::KeyEditor::keysChanged);
    QVERIFY(triggered.isValid());

    constexpr auto event = KeyEvent::ACCEPT;
    const auto old_key = AppSettings::keys.at(event).constFirst();
    constexpr auto new_key = Qt::Key_X;
    QVERIFY(AppSettings::keys.at(event).count(old_key) == 1);
    QVERIFY(AppSettings::keys.at(event).count(new_key) == 0);

    QTest::ignoreMessage(QtInfoMsg, "Program settings saved");
    keyeditor.changeKey(static_cast<int>(event), old_key, new_key);

    QCOMPARE(triggered.count(), 1);
    QVERIFY(AppSettings::keys.at(event).count(old_key) == 0);
    QVERIFY(AppSettings::keys.at(event).count(new_key) == 1);
}

void test_Keys::change2()
{
    model::KeyEditor keyeditor;

    QSignalSpy triggered(&keyeditor, &model::KeyEditor::keysChanged);
    QVERIFY(triggered.isValid());

    constexpr auto event1 = KeyEvent::ACCEPT;
    constexpr auto event2 = KeyEvent::CANCEL;
    const auto old_key = AppSettings::keys.at(event1).constFirst();
    const auto new_key = AppSettings::keys.at(event2).constFirst();
    QVERIFY(AppSettings::keys.at(event1).count(old_key) == 1);
    QVERIFY(AppSettings::keys.at(event1).count(new_key) == 0);
    QVERIFY(AppSettings::keys.at(event2).count(old_key) == 0);
    QVERIFY(AppSettings::keys.at(event2).count(new_key) == 1);

    QTest::ignoreMessage(QtInfoMsg, "Program settings saved");
    keyeditor.changeKey(static_cast<int>(event1), old_key, new_key);

    QCOMPARE(triggered.count(), 1);
    QVERIFY(AppSettings::keys.at(event1).count(old_key) == 0);
    QVERIFY(AppSettings::keys.at(event1).count(new_key) == 1);
    QVERIFY(AppSettings::keys.at(event2).count(old_key) == 0);
    QVERIFY(AppSettings::keys.at(event2).count(new_key) == 0);
}

void test_Keys::keyList()
{
    constexpr auto event = KeyEvent::ACCEPT;
    model::KeyEditor keyeditor;

    QCOMPARE(keyeditor.keysOf(static_cast<int>(event)), AppSettings::keys.at(event));
}

void test_Keys::keyName()
{
    model::KeyEditor keyeditor;

    QCOMPARE(keyeditor.keyName(Qt::Key_X), QStringLiteral("X"));
    QCOMPARE(keyeditor.keyName(GamepadKeyId::A), QStringLiteral("Gamepad 0 (A)"));
}


QTEST_MAIN(test_Keys)
#include "test_Keys.moc"
