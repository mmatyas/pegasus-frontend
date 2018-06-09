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

#include "providers/pegasus/PegasusProvider.h"
#include "modeldata/gaming/Collection.h"
#include "modeldata/gaming/Game.h"
#include "utils/HashMap.h"

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
    HashMap<QString, modeldata::GamePtr> games;
    HashMap<QString, modeldata::Collection> collections;

    providers::pegasus::PegasusProvider provider;
    provider.add_game_dir(QStringLiteral(":/empty"));
    provider.find(games, collections);

    QCOMPARE(collections.size(), 0ul);
    QCOMPARE(games.size(), 0ul);
}

void test_PegasusProvider::find_in_filled_dir()
{
    HashMap<QString, modeldata::GamePtr> games;
    HashMap<QString, modeldata::Collection> collections;

    QTest::ignoreMessage(QtInfoMsg, "Found `:/filled/collections.txt`");
    providers::pegasus::PegasusProvider provider;
    provider.add_game_dir(QStringLiteral(":/filled"));
    provider.find(games, collections);

    // finds the correct collections
    QCOMPARE(collections.size(), 3ul);
    QVERIFY(collections.count(QStringLiteral("My Games")));
    QVERIFY(collections.count(QStringLiteral("Favorite games")));
    QVERIFY(collections.count(QStringLiteral("Multi-game ROMs")));

    // finds the correct amount of games
    QCOMPARE(games.size(), 8ul);
    QCOMPARE(collections.at(QStringLiteral("My Games")).games().size(), 8ul);
    QCOMPARE(collections.at(QStringLiteral("Favorite games")).games().size(), 3ul);
    QCOMPARE(collections.at(QStringLiteral("Multi-game ROMs")).games().size(), 1ul);

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
    for (const modeldata::GamePtr& game : collections.at(QStringLiteral("My Games")).games()) {
        QVERIFY(game);
        QCOMPARE(mygames_paths.count(game->fileinfo().filePath()), 1);
    }
    for (const modeldata::GamePtr& game : collections.at(QStringLiteral("Favorite games")).games()) {
        QVERIFY(game);
        QCOMPARE(faves_paths.count(game->fileinfo().filePath()), 1);
    }
    for (const modeldata::GamePtr& game : collections.at(QStringLiteral("Multi-game ROMs")).games()) {
        QVERIFY(game);
        QCOMPARE(multi_paths.count(game->fileinfo().filePath()), 1);
    }

    // finds the correct games in general
    for (const auto& pair : games) {
        QCOMPARE(pair.second.isNull(), false);
        //QCOMPARE(mygames_paths.count(pair.second->fileinfo().filePath()), 1);
    }
}

void test_PegasusProvider::enhance()
{
    HashMap<QString, modeldata::GamePtr> games;
    HashMap<QString, modeldata::Collection> collections;

    QTest::ignoreMessage(QtInfoMsg, "Found `:/with_meta/collections.txt`");
    QTest::ignoreMessage(QtInfoMsg, "Found `:/with_meta/metadata.txt`");
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("`:/with_meta/metadata.txt`, line \\d: no file defined yet.*"));
    providers::pegasus::PegasusProvider provider;
    provider.add_game_dir(QStringLiteral(":/with_meta"));
    provider.find(games, collections);
    provider.enhance(games, collections);

    QCOMPARE(collections.size(), 1ul);
    QCOMPARE(games.size(), 4ul);

    modeldata::GamePtr game;

    game = games[QStringLiteral(":/with_meta/mygame1.ext")];
    QVERIFY(game);
    QCOMPARE(game->title, QStringLiteral("My Game 1"));
    QCOMPARE(game->developer_str, QStringLiteral("Dev1, Dev2"));
    QCOMPARE(game->developer_list, QStringList({"Dev1", "Dev2"}));

    game = games[QStringLiteral(":/with_meta/mygame2.ext")];
    QVERIFY(game);
    QCOMPARE(game->title, QStringLiteral("My Game 2"));
    QCOMPARE(game->publisher_str, QStringLiteral("Publisher with Spaces, Another Publisher"));
    QCOMPARE(game->publisher_list, QStringList({"Publisher with Spaces", "Another Publisher"}));

    game = games[QStringLiteral(":/with_meta/mygame3.ext")];
    QVERIFY(game);
    QCOMPARE(game->title, QStringLiteral("mygame3"));
    QCOMPARE(game->genre_str, QStringLiteral("genre1, genre2, genre with spaces"));
    QCOMPARE(game->genre_list, QStringList({"genre1", "genre2", "genre with spaces"}));
    QCOMPARE(game->player_count, 4);

    game = games[QStringLiteral(":/with_meta/subdir/game_in_subdir.ext")];
    QVERIFY(game);
    QCOMPARE(game->title, QStringLiteral("game_in_subdir"));
    QCOMPARE(game->rating, 0.8f);
    QCOMPARE(game->release_date, QDate(1998, 5, 1));
}


QTEST_MAIN(test_PegasusProvider)
#include "test_PegasusProvider.moc"
