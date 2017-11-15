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

#include "types/Filters.h"
#include "types/GameAssets.h"
#include "types/Platform.h"


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
    void prepareTestPlatform(Types::Platform&, QSignalSpy&, QSignalSpy&);
};

void test_Model::platformNullGame()
{
    Types::Platform platform("dummy", {"dummy"}, {"dummy"}, "dummy");

    // for some reason, using simply nullptr causes a build error (Qt 5.7)

    QCOMPARE(platform.gameList().current(), static_cast<Types::Game*>(nullptr));
    QCOMPARE(platform.gameList().index(), -1);
}

void test_Model::prepareTestPlatform(Types::Platform& platform,
                                     QSignalSpy& spy_idx, QSignalSpy& spy_game)
{
    // add two games, then lock the list;
    // after the lock, the platform should point to the first game
    // and trigger only once

    QVERIFY(spy_idx.isValid());
    QVERIFY(spy_game.isValid());

    platform.gameListMut().addGame("dummy0");
    platform.gameListMut().addGame("dummy1");
    platform.gameListMut().lockGameList();

    QVERIFY(platform.gameList().games().count() == 2);
    QVERIFY(platform.gameList().current() != static_cast<Types::Game*>(nullptr));
    QVERIFY(platform.gameList().current() == platform.gameList().games().first());
    QVERIFY(platform.gameList().index() == 0);
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

    Types::Platform platform("dummy", {"dummy"}, {"dummy"}, "dummy");
    Types::GameList& gameList = platform.gameListMut();
    QSignalSpy index_triggered(&gameList, &Types::GameList::currentChanged);
    QSignalSpy game_triggered(&gameList, &Types::GameList::currentChanged);

    prepareTestPlatform(platform, index_triggered, game_triggered);

    // test

    QFETCH(int, index);
    QFETCH(bool, game_change_triggered);

    if (index < -1 || index > gameList.games().count())
        QTest::ignoreMessage(QtWarningMsg, QRegularExpression("Invalid game index #-?[0-9]+"));

    gameList.setIndex(index);

    if (index == -1) {
        // -1 makes it undefined
        QCOMPARE(gameList.current(), static_cast<Types::Game*>(nullptr));
        QCOMPARE(gameList.index(), -1);
    }
    else {
        int expected_idx = game_change_triggered ? index : 0;

        QCOMPARE(gameList.current(), gameList.games().at(expected_idx));
        QCOMPARE(gameList.index(), expected_idx);
    }
    QCOMPARE(index_triggered.count(), game_change_triggered ? 2 : 1);
    QCOMPARE(game_triggered.count(), game_change_triggered ? 2 : 1);
}

void test_Model::platformAppendGame()
{
    Types::Platform platform("dummy", {"dummy"}, {"dummy"}, "dummy");
    Types::GameList& gameList = platform.gameListMut();
    QVERIFY(gameList.games().isEmpty());

    gameList.addGame("a");
    gameList.addGame("b");
    gameList.addGame("c");
    gameList.lockGameList();

    QCOMPARE(gameList.games().count(), 3);
}

void test_Model::platformSortGames()
{
    Types::Platform platform("dummy", {"dummy"}, {"dummy"}, "dummy");
    Types::GameList& gameList = platform.gameListMut();
    QVERIFY(gameList.games().isEmpty());

    gameList.addGame("bbb");
    gameList.addGame("aaa");
    gameList.sortGames();
    gameList.lockGameList();

    QCOMPARE(gameList.games().first()->m_rom_path, QLatin1String("aaa"));
    QCOMPARE(gameList.games().last()->m_rom_path, QLatin1String("bbb"));
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
    Types::Platform platform("dummy", {"dummy"}, {"dummy"}, "dummy");
    Types::GameList& gameList = platform.gameListMut();
    QSignalSpy triggered(&gameList, &Types::GameList::filteredGamesChanged);
    QVERIFY(triggered.isValid());

    gameList.addGame("file1");
        gameList.allGames().last()->m_title = "not-fav, 1P";
        gameList.allGames().last()->m_favorite = false;
        gameList.allGames().last()->m_players = 1;
    gameList.addGame("file2");
        gameList.allGames().last()->m_title = "not-fav, 2P";
        gameList.allGames().last()->m_favorite = false;
        gameList.allGames().last()->m_players = 2;
    gameList.addGame("file3");
        gameList.allGames().last()->m_title = "fav, 1P";
        gameList.allGames().last()->m_favorite = true;
        gameList.allGames().last()->m_players = 1;
    gameList.addGame("file4");
        gameList.allGames().last()->m_title = "My Game";
        gameList.allGames().last()->m_favorite = false;
        gameList.allGames().last()->m_players = 1;
    gameList.addGame("file5");
        gameList.allGames().last()->m_title = "Another Game";
        gameList.allGames().last()->m_favorite = true;
        gameList.allGames().last()->m_players = 1;

    QVERIFY(triggered.count() == 0);
    gameList.lockGameList();

    QVERIFY(gameList.games().count() == 5);
    QVERIFY(gameList.games().count() == gameList.allGames().count());
    QVERIFY(triggered.count() == 1);

    QFETCH(QString, title);
    QFETCH(bool, favorite);
    QFETCH(int, player_cnt);
    QFETCH(int, matching_games_cnt);

    Types::Filters filters;
        filters.m_title = title;
        filters.m_favorite = favorite;
        filters.m_player_count = player_cnt;

    gameList.applyFilters(filters);

    QCOMPARE(gameList.games().count(), matching_games_cnt);

    // if the filter didn't change the list of games,
    // there should be no new trigger
    if (matching_games_cnt == gameList.allGames().count())
        QCOMPARE(triggered.count(), 1);
    else
        QCOMPARE(triggered.count(), 2);
}

void test_Model::assetsSetSingle()
{
    Types::GameAssets assets;
    QCOMPARE(assets.boxFront(), QString());

    assets.setSingle(AssetType::BOX_FRONT, QUrl::fromLocalFile("/dummy").toString());
    QCOMPARE(assets.boxFront(), QLatin1String("file:///dummy"));
}

void test_Model::assetsAppendMulti()
{
    Types::GameAssets assets;
    QCOMPARE(assets.videos().count(), 0);

    assets.appendMulti(AssetType::VIDEOS, QUrl::fromLocalFile("/dummy").toString());
    QCOMPARE(assets.videos().count(), 1);
    QCOMPARE(assets.videos().constFirst(), QLatin1String("file:///dummy"));
}


QTEST_MAIN(test_Model)
#include "test_Model.moc"
