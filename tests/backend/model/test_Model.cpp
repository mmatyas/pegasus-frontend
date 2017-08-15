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

#include "Model.h"


class test_Model : public QObject
{
    Q_OBJECT

private slots:
    void platformNullGame();

    void platformSetIndex_data();
    void platformSetIndex();

    void platformChangeIndex_data();
    void platformChangeIndex();

    void platformAppendGame();
    void platformSortGames();

    void assetsSetSingle();
    void assetsAppendMulti();
};

void test_Model::platformNullGame()
{
    Model::Platform platform("dummy", "dummy", {"dummy"}, "dummy");

    // for some reason, using simply nullptr causes a build error (Qt 5.7)

    QCOMPARE(platform.currentGame(), static_cast<Model::Game*>(nullptr));
    QCOMPARE(platform.currentGameIndex(), -1);
}

void test_Model::platformSetIndex_data()
{
    QTest::addColumn<int>("index");
    QTest::addColumn<bool>("game_valid");
    QTest::addColumn<bool>("triggered");

    QTest::newRow("undefined (-1)") << -1 << false << true;
    QTest::newRow("valid (0)") << 0 << true << false;
    QTest::newRow("out of range (pos)") << 999 << true << false;
    QTest::newRow("out of range (neg)") << -999 << true << false;
}

void test_Model::platformSetIndex()
{
    // prepare

    Model::Platform platform("dummy", "dummy", {"dummy"}, "dummy");
    QSignalSpy index_triggered(&platform, &Model::Platform::currentGameIndexChanged);
    QSignalSpy game_triggered(&platform, &Model::Platform::currentGameChanged);
    QVERIFY(index_triggered.isValid());
    QVERIFY(game_triggered.isValid());

    platform.addGame("dummy");
    platform.lockGameList();

    QVERIFY(platform.currentGame() != nullptr);
    QVERIFY(platform.currentGameIndex() == 0);
    QVERIFY(index_triggered.count() == 1);
    QVERIFY(game_triggered.count() == 1);

    // test

    QFETCH(int, index);
    QFETCH(bool, game_valid);
    QFETCH(bool, triggered);

    if (index != -1 && index != 0)
        QTest::ignoreMessage(QtWarningMsg, QRegularExpression("Invalid game index #-?[0-9]+"));

    platform.setCurrentGameIndex(index);

    QCOMPARE(platform.currentGame(), game_valid ? platform.games().last() : nullptr);
    QCOMPARE(platform.currentGameIndex(), game_valid ? 0 : -1);
    QCOMPARE(index_triggered.count(), triggered ? 2 : 1);
    QCOMPARE(game_triggered.count(), triggered ? 2 : 1);
}

void test_Model::platformChangeIndex_data()
{
    QTest::addColumn<int>("index");
    QTest::addColumn<bool>("game_valid");
    QTest::addColumn<bool>("triggered");

    QTest::newRow("same") << 0 << true << false;
    QTest::newRow("reset") << -1 << false << true;
    QTest::newRow("out of range (pos)") << 999 << true << false;
    QTest::newRow("out of range (neg)") << -999 << true << false;
}

void test_Model::platformChangeIndex()
{
    // prepare

    Model::Platform platform("dummy", "dummy", {"dummy"}, "dummy");
    QSignalSpy index_triggered(&platform, &Model::Platform::currentGameIndexChanged);
    QSignalSpy game_triggered(&platform, &Model::Platform::currentGameChanged);
    QVERIFY(index_triggered.isValid());
    QVERIFY(game_triggered.isValid());

    platform.addGame("dummy");
    platform.lockGameList();

    QVERIFY(platform.currentGame() != nullptr);
    QVERIFY(platform.currentGameIndex() == 0);
    QVERIFY(index_triggered.count() == 1);
    QVERIFY(game_triggered.count() == 1);

    // test

    QFETCH(int, index);
    QFETCH(bool, game_valid);
    QFETCH(bool, triggered);

    if (index != -1 && index != 0)
        QTest::ignoreMessage(QtWarningMsg, QRegularExpression("Invalid game index #-?[0-9]+"));

    platform.setCurrentGameIndex(index);

    QCOMPARE(platform.currentGame(), game_valid ? platform.games().last() : nullptr);
    QCOMPARE(platform.currentGameIndex(), game_valid ? 0 : -1);
    QCOMPARE(index_triggered.count(), triggered ? 2 : 1);
    QCOMPARE(game_triggered.count(), triggered ? 2 : 1);
}

void test_Model::platformAppendGame()
{
    Model::Platform platform("dummy", "dummy", {"dummy"}, "dummy");
    QVERIFY(platform.games().isEmpty());

    platform.addGame("a");
    platform.addGame("b");
    platform.addGame("c");
    platform.lockGameList();

    QCOMPARE(platform.games().count(), 3);
}

void test_Model::platformSortGames()
{
    Model::Platform platform("dummy", "dummy", {"dummy"}, "dummy");
    QVERIFY(platform.games().isEmpty());

    platform.addGame("bbb");
    platform.addGame("aaa");
    platform.sortGames();
    platform.lockGameList();

    QCOMPARE(platform.games().first()->m_rom_path, QLatin1String("aaa"));
    QCOMPARE(platform.games().last()->m_rom_path, QLatin1String("bbb"));
}

void test_Model::assetsSetSingle()
{
    Model::GameAssets assets;
    QCOMPARE(assets.boxFront(), QString());

    assets.setSingle(Assets::Type::BOX_FRONT, "dummy");
    QCOMPARE(assets.boxFront(), QLatin1String("dummy"));
}

void test_Model::assetsAppendMulti()
{
    Model::GameAssets assets;
    QCOMPARE(assets.videos().count(), 0);

    assets.appendMulti(Assets::Type::VIDEOS, "dummy");
    QCOMPARE(assets.videos().count(), 1);
    QCOMPARE(assets.videos().first(), QLatin1String("dummy"));
}


QTEST_MAIN(test_Model)
#include "test_Model.moc"
