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
    void asset_search();
    void custom_assets();
    void custom_directories();
};

void test_PegasusProvider::find_in_empty_dir()
{
    HashMap<QString, modeldata::Game> games;
    HashMap<QString, modeldata::Collection> collections;
    HashMap<QString, std::vector<QString>> collection_childs;

    providers::pegasus::PegasusProvider provider({QStringLiteral(":/empty")});
    provider.findLists(games, collections, collection_childs);

    QVERIFY(games.empty());
    QVERIFY(collections.empty());
    QVERIFY(collection_childs.empty());
}

void test_PegasusProvider::find_in_filled_dir()
{
    HashMap<QString, modeldata::Game> games;
    HashMap<QString, modeldata::Collection> collections;
    HashMap<QString, std::vector<QString>> collection_childs;

    QTest::ignoreMessage(QtInfoMsg, "Collections: found `:/filled/collections.txt`");
    providers::pegasus::PegasusProvider provider({QStringLiteral(":/filled")});
    provider.findLists(games, collections, collection_childs);

    // finds the correct collections
    QVERIFY(collections.size() == 3);
    QVERIFY(collections.count(QStringLiteral("My Games")));
    QVERIFY(collections.count(QStringLiteral("Favorite games")));
    QVERIFY(collections.count(QStringLiteral("Multi-game ROMs")));

    // finds the correct amount of games
    QVERIFY(games.size() == 8);
    QVERIFY(collection_childs.at(QStringLiteral("My Games")).size() == 8);
    QVERIFY(collection_childs.at(QStringLiteral("Favorite games")).size() == 3);
    QVERIFY(collection_childs.at(QStringLiteral("Multi-game ROMs")).size() == 1);

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
    for (const QString& game_key : collection_childs.at(QStringLiteral("My Games"))) {
        const modeldata::Game& game = games.at(game_key);
        QCOMPARE(mygames_paths.contains(game.fileinfo().filePath()), true);
    }
    for (const QString& game_key : collection_childs.at(QStringLiteral("Favorite games"))) {
        const modeldata::Game& game = games.at(game_key);
        QCOMPARE(faves_paths.contains(game.fileinfo().filePath()), true);
    }
    for (const QString& game_key : collection_childs.at(QStringLiteral("Multi-game ROMs"))) {
        const modeldata::Game& game = games.at(game_key);
        QCOMPARE(multi_paths.contains(game.fileinfo().filePath()), true);
    }
}

void test_PegasusProvider::enhance()
{
    HashMap<QString, modeldata::Game> games;
    HashMap<QString, modeldata::Collection> collections;
    HashMap<QString, std::vector<QString>> collection_childs;

    QTest::ignoreMessage(QtInfoMsg, "Collections: found `:/with_meta/collections.txt`");
    QTest::ignoreMessage(QtInfoMsg, "Collections: found `:/with_meta/metadata.txt`");
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("`:/with_meta/metadata.txt`, line \\d: no file defined yet.*"));
    providers::pegasus::PegasusProvider provider({QStringLiteral(":/with_meta")});
    provider.findLists(games, collections, collection_childs);
    provider.findStaticData(games, collections, collection_childs);

    const QString collection_name(QStringLiteral("mygames"));
    QVERIFY(collections.size() == 1);
    QVERIFY(collections.count(collection_name) == 1);
    QCOMPARE(collections.at(collection_name).summary, QStringLiteral("this is the summary"));
    QCOMPARE(collections.at(collection_name).description, QStringLiteral("this is the description"));

    QVERIFY(games.size() == 4);
    QString game_key;

    game_key = QStringLiteral(":/with_meta/mygame1.ext");
    QCOMPARE(games.count(game_key) > 0, true);
    QCOMPARE(games.at(game_key).title, QStringLiteral("My Game 1"));
    QCOMPARE(games.at(game_key).developers, QStringList({"Dev1", "Dev2"}));

    game_key = QStringLiteral(":/with_meta/mygame2.ext");
    QCOMPARE(games.count(game_key) > 0, true);
    QCOMPARE(games.at(game_key).title, QStringLiteral("My Game 2"));
    QCOMPARE(games.at(game_key).publishers, QStringList({"Publisher with Spaces", "Another Publisher"}));

    game_key = QStringLiteral(":/with_meta/mygame3.ext");
    QCOMPARE(games.count(game_key) > 0, true);
    QCOMPARE(games.at(game_key).title, QStringLiteral("mygame3"));
    QCOMPARE(games.at(game_key).genres, QStringList({"genre1", "genre2", "genre with spaces"}));
    QCOMPARE(games.at(game_key).player_count, 4);

    game_key = QStringLiteral(":/with_meta/subdir/game_in_subdir.ext");
    QCOMPARE(games.count(game_key) > 0, true);
    QCOMPARE(games.at(game_key).title, QStringLiteral("game_in_subdir"));
    QCOMPARE(games.at(game_key).rating, 0.8f);
    QCOMPARE(games.at(game_key).release_date, QDate(1998, 5, 1));
}

void test_PegasusProvider::asset_search()
{
    HashMap<QString, modeldata::Game> games;
    HashMap<QString, modeldata::Collection> collections;
    HashMap<QString, std::vector<QString>> collection_childs;

    providers::pegasus::PegasusProvider provider({QStringLiteral(":/asset_search")});

    QTest::ignoreMessage(QtInfoMsg, "Collections: found `:/asset_search/collections.txt`");
    provider.findLists(games, collections, collection_childs);
    provider.findStaticData(games, collections, collection_childs);

    const QString collection_name(QStringLiteral("mygames"));
    QVERIFY(collections.size() == 1);
    QVERIFY(collections.count(collection_name) == 1);
    QVERIFY(games.size() == 4);

    auto game_it = games.find(QStringLiteral(":/asset_search/mygame1.ext"));
    QVERIFY(game_it != games.cend());
    QCOMPARE(game_it->second.assets.single(AssetType::BOX_FRONT),
             QStringLiteral("file::/asset_search/media/mygame1/box_front.png"));
    QCOMPARE(game_it->second.assets.multi(AssetType::VIDEOS),
             { QStringLiteral("file::/asset_search/media/mygame1/video.mp4") });

    game_it = games.find(QStringLiteral(":/asset_search/mygame3.ext"));
    QVERIFY(game_it != games.cend());
    QCOMPARE(game_it->second.assets.multi(AssetType::SCREENSHOTS),
             { QStringLiteral("file::/asset_search/media/mygame3/screenshot.jpg") });
    QCOMPARE(game_it->second.assets.single(AssetType::MUSIC),
             QStringLiteral("file::/asset_search/media/mygame3/music.mp3"));

    game_it = games.find(QStringLiteral(":/asset_search/subdir/mygame4.ext"));
    QVERIFY(game_it != games.cend());
    QCOMPARE(game_it->second.assets.single(AssetType::BACKGROUND),
             QStringLiteral("file::/asset_search/media/subdir/mygame4/background.png"));
}

void test_PegasusProvider::custom_assets()
{
    HashMap<QString, modeldata::Game> games;
    HashMap<QString, modeldata::Collection> collections;
    HashMap<QString, std::vector<QString>> collection_childs;

    QTest::ignoreMessage(QtInfoMsg, "Collections: found `:/custom_assets/collections.txt`");
    QTest::ignoreMessage(QtInfoMsg, "Collections: found `:/custom_assets/metadata.txt`");
    providers::pegasus::PegasusProvider provider({QStringLiteral(":/custom_assets")});
    provider.findLists(games, collections, collection_childs);
    provider.findStaticData(games, collections, collection_childs);

    QVERIFY(collections.size() == 1);
    QVERIFY(games.size() == 1);

    const QString game_key = QStringLiteral(":/custom_assets/mygame1.ext");
    QCOMPARE(games.count(game_key) > 0, true);
    QCOMPARE(games.at(game_key).assets.single(AssetType::BOX_FRONT),
             QStringLiteral("file::/custom_assets/different_dir/whatever.png"));
}

void test_PegasusProvider::custom_directories()
{
    HashMap<QString, modeldata::Game> games;
    HashMap<QString, modeldata::Collection> collections;
    HashMap<QString, std::vector<QString>> collection_childs;

    QTest::ignoreMessage(QtInfoMsg, "Collections: found `:/custom_dirs/coll/collections.txt`");
    providers::pegasus::PegasusProvider provider({QStringLiteral(":/custom_dirs/coll")});
    provider.findLists(games, collections, collection_childs);
    provider.findStaticData(games, collections, collection_childs);

    QVERIFY(collections.size() == 2);
    QVERIFY(collections.count(QStringLiteral("x-files")) == 1);
    QVERIFY(collections.count(QStringLiteral("y-files")) == 1);
    QVERIFY(games.size() == 5);
    QVERIFY(collection_childs.at(QStringLiteral("x-files")).size() == 4);
    QVERIFY(collection_childs.at(QStringLiteral("y-files")).size() == 1);

    // NOTE: Apparently canonicalFilePath doesn't work with QRC resources...
    const QStringList xfiles {
        { ":/custom_dirs/coll/../a/mygame.x" },
        { ":/custom_dirs/coll/../b/mygame.x" },
        { ":/custom_dirs/c/mygame.x" },
        { ":/custom_dirs/coll/mygame.x" },
    };
    const QStringList yfiles {
        { ":/custom_dirs/coll/../b/mygame.y" },
    };

    for (const QString& game_key : collection_childs.at(QStringLiteral("x-files"))) {
        const modeldata::Game& game = games.at(game_key);
        QCOMPARE(xfiles.contains(game.fileinfo().canonicalFilePath()), true);
    }
    for (const QString& game_key : collection_childs.at(QStringLiteral("y-files"))) {
        const modeldata::Game& game = games.at(game_key);
        QCOMPARE(yfiles.contains(game.fileinfo().canonicalFilePath()), true);
    }
}


QTEST_MAIN(test_PegasusProvider)
#include "test_PegasusProvider.moc"
