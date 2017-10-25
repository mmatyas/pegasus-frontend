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

#include "api_parts/ApiFilters.h"
#include "model/Platform.h"


class test_Model : public QObject
{
    Q_OBJECT

private slots:
    void platformNullGame();

    void platformSetIndex_data();
    void platformSetIndex();

    void platformAppendGame();
    void platformSortGames();

    void platformApplyFilters_data();
    void platformApplyFilters();

    void assetsSetSingle();
    void assetsAppendMulti();

private:
    void prepareTestPlatform(Model::Platform&, QSignalSpy&, QSignalSpy&);
};

void test_Model::platformNullGame()
{
    Model::Platform platform("dummy", {"dummy"}, {"dummy"}, "dummy");

    // for some reason, using simply nullptr causes a build error (Qt 5.7)

    QCOMPARE(platform.currentGame(), static_cast<Model::Game*>(nullptr));
    QCOMPARE(platform.currentGameIndex(), -1);
}

void test_Model::prepareTestPlatform(Model::Platform& platform,
                                     QSignalSpy& spy_idx, QSignalSpy& spy_game)
{
    // add two games, then lock the list;
    // after the lock, the platform should point to the first game
    // and trigger only once

    QVERIFY(spy_idx.isValid());
    QVERIFY(spy_game.isValid());

    platform.addGame("dummy0");
    platform.addGame("dummy1");
    platform.lockGameList();

    QVERIFY(platform.games().count() == 2);
    QVERIFY(platform.currentGame() != static_cast<Model::Game*>(nullptr));
    QVERIFY(platform.currentGame() == platform.games().first());
    QVERIFY(platform.currentGameIndex() == 0);
    QVERIFY(spy_idx.count() == 1);
    QVERIFY(spy_game.count() == 1);
}

void test_Model::platformSetIndex_data()
{
    QTest::addColumn<int>("index");
    QTest::addColumn<bool>("game_change_triggered");

    QTest::newRow("undefined (-1)") << -1 << true;
    QTest::newRow("first/same (0)") << 0 << false;
    QTest::newRow("second/different (1)") << 1 << true;
    QTest::newRow("out of range (pos)") << 999 << false;
    QTest::newRow("out of range (neg)") << -999 << false;
}

void test_Model::platformSetIndex()
{
    // prepare

    Model::Platform platform("dummy", {"dummy"}, {"dummy"}, "dummy");
    QSignalSpy index_triggered(&platform, &Model::Platform::currentGameIndexChanged);
    QSignalSpy game_triggered(&platform, &Model::Platform::currentGameChanged);

    prepareTestPlatform(platform, index_triggered, game_triggered);

    // test

    QFETCH(int, index);
    QFETCH(bool, game_change_triggered);

    if (index < -1 || index > platform.games().count())
        QTest::ignoreMessage(QtWarningMsg, QRegularExpression("Invalid game index #-?[0-9]+"));

    platform.setCurrentGameIndex(index);

    if (index == -1) {
        // -1 makes it undefined
        QCOMPARE(platform.currentGame(), static_cast<Model::Game*>(nullptr));
        QCOMPARE(platform.currentGameIndex(), -1);
    }
    else {
        int expected_idx = game_change_triggered ? index : 0;

        QCOMPARE(platform.currentGame(), platform.games().at(expected_idx));
        QCOMPARE(platform.currentGameIndex(), expected_idx);
    }
    QCOMPARE(index_triggered.count(), game_change_triggered ? 2 : 1);
    QCOMPARE(game_triggered.count(), game_change_triggered ? 2 : 1);
}

void test_Model::platformAppendGame()
{
    Model::Platform platform("dummy", {"dummy"}, {"dummy"}, "dummy");
    QVERIFY(platform.games().isEmpty());

    platform.addGame("a");
    platform.addGame("b");
    platform.addGame("c");
    platform.lockGameList();

    QCOMPARE(platform.games().count(), 3);
}

void test_Model::platformSortGames()
{
    Model::Platform platform("dummy", {"dummy"}, {"dummy"}, "dummy");
    QVERIFY(platform.games().isEmpty());

    platform.addGame("bbb");
    platform.addGame("aaa");
    platform.sortGames();
    platform.lockGameList();

    QCOMPARE(platform.games().first()->m_rom_path, QLatin1String("aaa"));
    QCOMPARE(platform.games().last()->m_rom_path, QLatin1String("bbb"));
}

void test_Model::platformApplyFilters_data()
{
    QTest::addColumn<QString>("title");
    QTest::addColumn<bool>("favorite");
    QTest::addColumn<int>("player_cnt");
    QTest::addColumn<int>("matching_games_cnt");

    QTest::newRow("empty") << QString("") << false << 1 << 5;
    QTest::newRow("full title") << "My Game" << false << 1 << 1;
    QTest::newRow("partial title") << "Game" << false << 1 << 2;
    QTest::newRow("favorite") << "" << true << 1 << 2;
    QTest::newRow("multiplayer") << "" << false << 2 << 1;
    QTest::newRow("title + favorite") << "Game" << true << 1 << 1;
    QTest::newRow("title + favorite + 2P") << "Game" << true << 2 << 0;
}

void test_Model::platformApplyFilters()
{
    Model::Platform platform("dummy", {"dummy"}, {"dummy"}, "dummy");
    QSignalSpy triggered(&platform, &Model::Platform::filteredGamesChanged);
    QVERIFY(triggered.isValid());

    platform.addGame("file1");
        platform.allGames().last()->m_title = "not-fav, 1P";
        platform.allGames().last()->m_favorite = false;
        platform.allGames().last()->m_players = 1;
    platform.addGame("file2");
        platform.allGames().last()->m_title = "not-fav, 2P";
        platform.allGames().last()->m_favorite = false;
        platform.allGames().last()->m_players = 2;
    platform.addGame("file3");
        platform.allGames().last()->m_title = "fav, 1P";
        platform.allGames().last()->m_favorite = true;
        platform.allGames().last()->m_players = 1;
    platform.addGame("file4");
        platform.allGames().last()->m_title = "My Game";
        platform.allGames().last()->m_favorite = false;
        platform.allGames().last()->m_players = 1;
    platform.addGame("file5");
        platform.allGames().last()->m_title = "Another Game";
        platform.allGames().last()->m_favorite = true;
        platform.allGames().last()->m_players = 1;

    QVERIFY(triggered.count() == 0);
    platform.lockGameList();

    QVERIFY(platform.games().count() == 5);
    QVERIFY(platform.games().count() == platform.allGames().count());
    QVERIFY(triggered.count() == 1);

    QFETCH(QString, title);
    QFETCH(bool, favorite);
    QFETCH(int, player_cnt);
    QFETCH(int, matching_games_cnt);

    ApiParts::Filters filters;
        filters.m_title = title;
        filters.m_favorite = favorite;
        filters.m_player_count = player_cnt;

    platform.applyFilters(filters);

    QCOMPARE(platform.games().count(), matching_games_cnt);

    // if the filter didn't change the list of games,
    // there should be no new trigger
    if (matching_games_cnt == platform.allGames().count())
        QCOMPARE(triggered.count(), 1);
    else
        QCOMPARE(triggered.count(), 2);
}

void test_Model::assetsSetSingle()
{
    Model::GameAssets assets;
    QCOMPARE(assets.boxFront(), QString());

    assets.setSingle(AssetType::BOX_FRONT, "dummy");
    QCOMPARE(assets.boxFront(), QLatin1String("dummy"));
}

void test_Model::assetsAppendMulti()
{
    Model::GameAssets assets;
    QCOMPARE(assets.videos().count(), 0);

    assets.appendMulti(AssetType::VIDEOS, "dummy");
    QCOMPARE(assets.videos().count(), 1);
    QCOMPARE(assets.videos().constFirst(), QLatin1String("dummy"));
}


QTEST_MAIN(test_Model)
#include "test_Model.moc"
