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

#include "types/Collection.h"
#include "types/Game.h"
#include "providers/pegasus/PegasusProvider.h"
#include <QHash>
#include <QString>


class test_PegasusProvider : public QObject {
    Q_OBJECT

private slots:
    void find_in_empty_dir();
    void find_in_filled_dir();
    void enhance();
};

void test_PegasusProvider::find_in_empty_dir()
{
    QHash<QString, Types::Game*> games;
    QHash<QString, Types::Collection*> collections;

    providers::pegasus::PegasusProvider provider;
    provider.find_in_dirs({ QStringLiteral(":/empty") }, games, collections);

    QCOMPARE(collections.count(), 0);
    QCOMPARE(games.count(), 0);
}

void test_PegasusProvider::find_in_filled_dir()
{
    QHash<QString, Types::Game*> games;
    QHash<QString, Types::Collection*> collections;

    QTest::ignoreMessage(QtInfoMsg, "Found `:/filled/collections.txt`");
    providers::pegasus::PegasusProvider provider;
    provider.find_in_dirs({ QStringLiteral(":/filled") }, games, collections);

    // finds the correct collections
    QCOMPARE(collections.count(), 3);
    QVERIFY(collections[QStringLiteral("My Games")] != nullptr);
    QVERIFY(collections[QStringLiteral("Favorite games")] != nullptr);
    QVERIFY(collections[QStringLiteral("Multi-game ROMs")] != nullptr);

    // finds the correct amount of games
    QCOMPARE(games.count(), 8);
    QCOMPARE(collections[QStringLiteral("My Games")]->gameList().allCount(), 8);
    QCOMPARE(collections[QStringLiteral("Favorite games")]->gameList().allCount(), 3);
    QCOMPARE(collections[QStringLiteral("Multi-game ROMs")]->gameList().allCount(), 1);

    // finds the correct files for the collections
    const QStringList mygames_paths {
        { ":/filled/mygame1.ext" },
        { ":/filled/mygame2.ext" },
        { ":/filled/mygame3.ext" },
        { ":/filled/favgame1.ext" },
        { ":/filled/favgame2.ext" },
        { ":/filled/game with spaces.ext" },
        { ":/filled/9999-in-1.ext" },
        { ":/filled/subdir/game_in_subdir.ext" },
    };
    const QStringList faves_paths {
        { ":/filled/favgame1.ext" },
        { ":/filled/favgame2.ext" },
        { ":/filled/game with spaces.ext" },
    };
    const QStringList multi_paths {
        { ":/filled/9999-in-1.ext" },
    };
    for (const Types::Game* const game : collections[QStringLiteral("My Games")]->gameList().allGames()) {
        QVERIFY(game != nullptr);
        QCOMPARE(mygames_paths.count(game->m_fileinfo.filePath()), 1);
    }
    for (const Types::Game* const game : collections[QStringLiteral("Favorite games")]->gameList().allGames()) {
        QVERIFY(game != nullptr);
        QCOMPARE(faves_paths.count(game->m_fileinfo.filePath()), 1);
    }
    for (const Types::Game* const game : collections[QStringLiteral("Multi-game ROMs")]->gameList().allGames()) {
        QVERIFY(game != nullptr);
        QCOMPARE(multi_paths.count(game->m_fileinfo.filePath()), 1);
    }

    // finds the correct games in general
    const QStringList all_game_paths = mygames_paths;
    for (const Types::Game* const game : games.values()) {
        QVERIFY(game != nullptr);
        QCOMPARE(all_game_paths.count(game->m_fileinfo.filePath()), 1);
    }
}

void test_PegasusProvider::enhance()
{
    QHash<QString, Types::Game*> games;
    QHash<QString, Types::Collection*> collections;

    QTest::ignoreMessage(QtInfoMsg, "Found `:/with_meta/collections.txt`");
    QTest::ignoreMessage(QtInfoMsg, "Found `:/with_meta/metadata.txt`");
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("`:/with_meta/metadata.txt`, line \\d: no file defined yet.*"));
    providers::pegasus::PegasusProvider provider;
    provider.find_in_dirs({ QStringLiteral(":/with_meta") }, games, collections);
    provider.enhance_in_dirs({ QStringLiteral(":/with_meta") }, games, collections);

    QCOMPARE(collections.count(), 1);
    QCOMPARE(games.count(), 4);

    const Types::Game* game;

    game = games[QStringLiteral(":/with_meta/mygame1.ext")];
    QVERIFY(game);
    QCOMPARE(game->property("title").toString(), QStringLiteral("My Game 1"));
    QCOMPARE(game->property("developer").toString(), QStringLiteral("Dev1, Dev2"));
    QCOMPARE(game->property("developerList").toStringList(), QStringList({"Dev1", "Dev2"}));

    game = games[QStringLiteral(":/with_meta/mygame2.ext")];
    QVERIFY(game);
    QCOMPARE(game->property("title").toString(), QStringLiteral("My Game 2"));
    QCOMPARE(game->property("publisher").toString(), QStringLiteral("Publisher with Spaces, Another Publisher"));
    QCOMPARE(game->property("publisherList").toStringList(), QStringList({"Publisher with Spaces", "Another Publisher"}));

    game = games[QStringLiteral(":/with_meta/mygame3.ext")];
    QVERIFY(game);
    QCOMPARE(game->property("title").toString(), QStringLiteral("mygame3"));
    QCOMPARE(game->property("genre").toString(), QStringLiteral("genre1, genre2, genre with spaces"));
    QCOMPARE(game->property("genreList").toStringList(), QStringList({"genre1", "genre2", "genre with spaces"}));
    QCOMPARE(game->property("players").toInt(), 4);

    game = games[QStringLiteral(":/with_meta/subdir/game_in_subdir.ext")];
    QVERIFY(game);
    QCOMPARE(game->property("title").toString(), QStringLiteral("game_in_subdir"));
    QCOMPARE(game->property("rating").toFloat(), 0.8f);
    QCOMPARE(game->property("release").toDate(), QDate(1998, 5, 1));
}


QTEST_MAIN(test_PegasusProvider)
#include "test_PegasusProvider.moc"
