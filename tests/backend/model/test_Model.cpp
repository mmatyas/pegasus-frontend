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
#include "types/Platform.h"


class test_Model : public QObject
{
    Q_OBJECT

private slots:
    void platformSetIndex_data();
    void platformSetIndex();

    void assetsSetSingle();
    void assetsAppendMulti();

private:
    void prepareTestPlatform(Types::Platform&, QSignalSpy&, QSignalSpy&);
};

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

    QVERIFY(platform.gameList().filteredGames().count() == 2);
    QVERIFY(platform.gameList().current() != static_cast<Types::Game*>(nullptr));
    QVERIFY(platform.gameList().current() == platform.gameList().filteredGames().first());
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

    Types::Platform platform;
    Types::GameList& gameList = platform.gameListMut();
    QSignalSpy index_triggered(&gameList, &Types::GameList::currentChanged);
    QSignalSpy game_triggered(&gameList, &Types::GameList::currentChanged);

    prepareTestPlatform(platform, index_triggered, game_triggered);

    // test

    QFETCH(int, index);
    QFETCH(bool, game_change_triggered);

    if (index < -1 || index > gameList.filteredGames().count())
        QTest::ignoreMessage(QtWarningMsg, QRegularExpression("Invalid game index #-?[0-9]+"));

    gameList.setIndex(index);

    if (index == -1) {
        // -1 makes it undefined
        QCOMPARE(gameList.current(), static_cast<Types::Game*>(nullptr));
        QCOMPARE(gameList.index(), -1);
    }
    else {
        int expected_idx = game_change_triggered ? index : 0;

        QCOMPARE(gameList.current(), gameList.filteredGames().at(expected_idx));
        QCOMPARE(gameList.index(), expected_idx);
    }
    QCOMPARE(index_triggered.count(), game_change_triggered ? 2 : 1);
    QCOMPARE(game_triggered.count(), game_change_triggered ? 2 : 1);
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
