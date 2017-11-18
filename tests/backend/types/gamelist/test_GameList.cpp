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
#include "types/GameList.h"


class test_GameList : public QObject {
    Q_OBJECT

private slots:
    void empty();

    void addGame();
    void sortGames();

    void applyFilters();
    void applyFilters_data();
};

void test_GameList::empty()
{
    Types::GameList list;

    QCOMPARE(list.property("current").value<Types::Game*>(), nullptr);
    QCOMPARE(list.property("index").toInt(), -1);
    QCOMPARE(list.property("count").toInt(), 0);
    QCOMPARE(list.property("countAll").toInt(), 0);
}

void test_GameList::addGame()
{
    Types::GameList list;

    QVERIFY(list.allGames().isEmpty());
    QVERIFY(list.filteredGames().isEmpty());

    list.addGame("a");
    list.addGame("b");
    list.addGame("c");
    list.lockGameList();

    QCOMPARE(list.property("current").value<Types::Game*>(), list.allGames().first());
    QCOMPARE(list.property("index").toInt(), 0);
    QCOMPARE(list.property("count").toInt(), 3);
    QCOMPARE(list.property("countAll").toInt(), 3);
}

void test_GameList::sortGames()
{
    Types::GameList gamelist;

    QVERIFY(gamelist.allGames().isEmpty());

    gamelist.addGame("bbb");
    gamelist.addGame("aaa");
    gamelist.sortGames();
    gamelist.lockGameList();

    QCOMPARE(gamelist.allGames().first()->m_rom_path, QLatin1String("aaa"));
    QCOMPARE(gamelist.allGames().last()->m_rom_path, QLatin1String("bbb"));
}

void test_GameList::applyFilters()
{
    Types::GameList gamelist;
    QSignalSpy triggered(&gamelist, &Types::GameList::filteredGamesChanged);
    QVERIFY(triggered.isValid());

    gamelist.addGame("game0");
        gamelist.allGames().last()->m_title = "not-fav, 1P";
        gamelist.allGames().last()->m_favorite = false;
        gamelist.allGames().last()->m_players = 1;
    gamelist.addGame("game1");
        gamelist.allGames().last()->m_title = "not-fav, 2P";
        gamelist.allGames().last()->m_favorite = false;
        gamelist.allGames().last()->m_players = 2;
    gamelist.addGame("game2");
        gamelist.allGames().last()->m_title = "fav, 1P";
        gamelist.allGames().last()->m_favorite = true;
        gamelist.allGames().last()->m_players = 1;
    gamelist.addGame("game3");
        gamelist.allGames().last()->m_title = "My Game";
        gamelist.allGames().last()->m_favorite = false;
        gamelist.allGames().last()->m_players = 1;
    gamelist.addGame("game4");
        gamelist.allGames().last()->m_title = "Another Game";
        gamelist.allGames().last()->m_favorite = true;
        gamelist.allGames().last()->m_players = 1;

    QVERIFY(triggered.count() == 0);
    gamelist.lockGameList();
    QVERIFY(triggered.count() == 1);

    // just to make sure
    QVERIFY(gamelist.property("count").toInt() == 5);
    QVERIFY(gamelist.property("countAll").toInt() == 5);


    QFETCH(QString, title);
    QFETCH(bool, favorite);
    QFETCH(int, player_cnt);
    QFETCH(int, matching_games_cnt);

    Types::Filters filters;
        filters.m_title = title;
        filters.m_favorite = favorite;
        filters.m_player_count = player_cnt;

    gamelist.applyFilters(filters);
    QCOMPARE(gamelist.property("count").toInt(), matching_games_cnt);
    QCOMPARE(gamelist.property("countAll").toInt(), 5);

    // if the filter didn't change the list of games, there should be
    // no new trigger -- we only check the count here for simplicity
    if (matching_games_cnt == gamelist.allGames().count())
        QCOMPARE(triggered.count(), 1);
    else
        QCOMPARE(triggered.count(), 2);
}

void test_GameList::applyFilters_data()
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


QTEST_MAIN(test_GameList)
#include "test_GameList.moc"
