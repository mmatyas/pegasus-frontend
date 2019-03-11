// Pegasus Frontend
// Copyright (C) 2017-2019  Mátyás Mustoha
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
#include "modeldata/gaming/CollectionData.h"
#include "modeldata/gaming/GameData.h"
#include "utils/HashMap.h"

#include <QString>
#include <QStringList>


namespace {

int contains_path(const QString& path, const std::vector<modeldata::GameFile>& files)
{
    int contains = 0;
    for (const modeldata::GameFile& gf : files) {
        if (gf.fileinfo.filePath() == path)
            contains++;
    }
    return contains;
}

void verify_collected_files(providers::SearchContext& ctx, const HashMap<QString, QStringList>& coll_files_map)
{
    for (const auto& entry : coll_files_map) {
        const QString& coll_name = entry.first;
        const QStringList& coll_files = entry.second;

        std::vector<size_t> expected_ids;

        for (const QString& abs_path : coll_files) {
            const QString can_path = QFileInfo(abs_path).canonicalFilePath();
            QVERIFY(!can_path.isEmpty());

            QVERIFY(ctx.path_to_gameid.count(can_path));
            const size_t game_id = ctx.path_to_gameid.at(can_path);

            QVERIFY(contains_path(abs_path, ctx.games.at(game_id).files));
            expected_ids.emplace_back(game_id);
        }

        std::vector<size_t> actual_ids = ctx.collection_childs.at(coll_name);

        std::sort(actual_ids.begin(), actual_ids.end());
        std::sort(expected_ids.begin(), expected_ids.end());
        QCOMPARE(actual_ids, expected_ids);
    }
}

} // namespace


class test_PegasusProvider : public QObject {
    Q_OBJECT

private slots:
    void empty();
    void simple();
    void with_meta();
    void asset_search();
    void custom_assets();
    void custom_directories();
    void multifile();
};

void test_PegasusProvider::empty()
{
    providers::SearchContext ctx;

    QTest::ignoreMessage(QtWarningMsg, "Collections: No metadata file found in `:/empty`, directory ignored");
    providers::pegasus::PegasusProvider provider({QStringLiteral(":/empty")});
    provider.findLists(ctx);

    QVERIFY(ctx.games.empty());
    QVERIFY(ctx.collections.empty());
    QVERIFY(ctx.collection_childs.empty());
}

void test_PegasusProvider::simple()
{
    providers::SearchContext ctx;

    QTest::ignoreMessage(QtInfoMsg, "Collections: found `:/simple/metadata.pegasus.txt`");
    providers::pegasus::PegasusProvider provider({QStringLiteral(":/simple")});
    provider.findLists(ctx);

    // finds the correct collections
    QCOMPARE(static_cast<int>(ctx.collections.size()), 3);
    QVERIFY(ctx.collections.count(QStringLiteral("My Games")));
    QVERIFY(ctx.collections.count(QStringLiteral("Favorite games")));
    QVERIFY(ctx.collections.count(QStringLiteral("Multi-game ROMs")));

    // finds the correct amount of games
    QCOMPARE(static_cast<int>(ctx.games.size()), 8);
    QVERIFY(ctx.collection_childs.at(QStringLiteral("My Games")).size() == 8);
    QVERIFY(ctx.collection_childs.at(QStringLiteral("Favorite games")).size() == 3);
    QVERIFY(ctx.collection_childs.at(QStringLiteral("Multi-game ROMs")).size() == 1);

    // finds the correct files for the collections
    const HashMap<QString, QStringList> coll_files_map {
        { QStringLiteral("My Games"), {
            { ":/simple/mygame1.ext" },
            { ":/simple/mygame2.ext" },
            { ":/simple/mygame3.ext" },
            { ":/simple/favgame1.ext" },
            { ":/simple/favgame2.ext" },
            { ":/simple/game with spaces.ext" },
            { ":/simple/9999-in-1.ext" },
            { ":/simple/subdir/game_in_subdir.ext" },
        }},
        { QStringLiteral("Favorite games"), {
            { ":/simple/favgame1.ext" },
            { ":/simple/favgame2.ext" },
            { ":/simple/game with spaces.ext" },
        }},
        { QStringLiteral("Multi-game ROMs"), {
            { ":/simple/9999-in-1.ext" },
        }},
    };

    verify_collected_files(ctx, coll_files_map);
}

void test_PegasusProvider::with_meta()
{
    providers::SearchContext ctx;

    QTest::ignoreMessage(QtInfoMsg, "Collections: found `:/with_meta/metadata.pegasus.txt`");
    QTest::ignoreMessage(QtWarningMsg, "Collections: `:/with_meta/metadata.pegasus.txt`, line 60: duplicate file `horse.ext`");
    QTest::ignoreMessage(QtWarningMsg, "Collections: `:/with_meta/metadata.pegasus.txt`, line 63: failed to parse rating value");
    QTest::ignoreMessage(QtWarningMsg, "Collections: `:/with_meta/metadata.pegasus.txt`, line 65: incorrect date format, should be YYYY, YYYY-MM or YYYY-MM-DD");
    QTest::ignoreMessage(QtWarningMsg, "Collections: `:/with_meta/metadata.pegasus.txt`, line 66: incorrect date format, should be YYYY, YYYY-MM or YYYY-MM-DD");
    QTest::ignoreMessage(QtWarningMsg, "Collections: `:/with_meta/metadata.pegasus.txt`, line 68: duplicate file `horse.ext`");
    QTest::ignoreMessage(QtWarningMsg, "Collections: `:/with_meta/metadata.pegasus.txt`, line 69: unrecognized game property `asd`, ignored");

    providers::pegasus::PegasusProvider provider({QStringLiteral(":/with_meta")});
    provider.findLists(ctx);

    QCOMPARE(static_cast<int>(ctx.collections.size()), 1);
    QCOMPARE(static_cast<int>(ctx.games.size()), 6);

    const auto common_launch = QStringLiteral("launcher.exe \"{file.path}\"");
    const auto common_workdir = QStringLiteral("some/workdir");

    // Collection
    {
        const QString collection_name(QStringLiteral("My Games"));

        QVERIFY(ctx.collections.count(collection_name));
        const modeldata::Collection& coll = ctx.collections.at(collection_name);

        QCOMPARE(coll.shortName(), QStringLiteral("mygames"));
        QCOMPARE(coll.summary, QStringLiteral("this is the summary"));
        QCOMPARE(coll.description, QStringLiteral("this is the description"));
        QCOMPARE(coll.launch_cmd, common_launch);
        QCOMPARE(coll.launch_workdir, common_workdir);
    }

    // Game before the first collection entry
    {
        const QString file_path = QStringLiteral(":/with_meta/pre.ext");

        QVERIFY(ctx.path_to_gameid.count(file_path));
        const size_t game_id = ctx.path_to_gameid.at(file_path);

        QVERIFY(ctx.games.count(game_id));
        const modeldata::Game& game = ctx.games.at(game_id);

        QCOMPARE(game.launch_cmd, common_launch);
        QCOMPARE(game.launch_workdir, common_workdir);
    }

    // Basic
    {
        const QString file_path = QStringLiteral(":/with_meta/basic.ext");

        QVERIFY(ctx.path_to_gameid.count(file_path));
        const size_t game_id = ctx.path_to_gameid.at(file_path);

        QVERIFY(ctx.games.count(game_id));
        const modeldata::Game& game = ctx.games.at(game_id);

        QCOMPARE(game.title, QStringLiteral("A simple game"));
        QCOMPARE(game.developers, QStringList({"Dev", "Dev with Spaces"}));
        QCOMPARE(game.publishers, QStringList({"The Company"}));
        QCOMPARE(game.genres, QStringList({"genre1", "genre2", "genre with spaces"}));
        QCOMPARE(static_cast<int>(game.player_count), 4);
        QCOMPARE(game.release_date, QDate(1998, 5, 1));
        QCOMPARE(game.summary, QStringLiteral("something short here"));
        QCOMPARE(game.description, QStringLiteral("a very long\ndescription"));
        QCOMPARE(game.rating, 0.8f);
        QCOMPARE(game.launch_cmd, common_launch);
        QCOMPARE(game.launch_workdir, common_workdir);
        QCOMPARE(static_cast<int>(game.files.size()), 1);
        QCOMPARE(contains_path(file_path, game.files), 1);
    }

    // Subdir
    {
        const QString file_path = QStringLiteral(":/with_meta/subdir/game_in_subdir.ext");

        QVERIFY(ctx.path_to_gameid.count(file_path));
        const size_t game_id = ctx.path_to_gameid.at(file_path);

        QVERIFY(ctx.games.count(game_id));
        const modeldata::Game& game = ctx.games.at(game_id);

        QCOMPARE(game.title, QStringLiteral("Subdir Game"));
        QCOMPARE(game.launch_cmd, common_launch);
        QCOMPARE(game.launch_workdir, common_workdir);
    }

    // Multifile
    {
        const QString file_path_a = QStringLiteral(":/with_meta/multi.a.ext");
        const QString file_path_b = QStringLiteral(":/with_meta/multi.b.ext");
        QVERIFY(ctx.path_to_gameid.count(file_path_a));
        QVERIFY(ctx.path_to_gameid.count(file_path_b));
        const size_t game_id_a = ctx.path_to_gameid.at(file_path_a);
        const size_t game_id_b = ctx.path_to_gameid.at(file_path_b);
        QCOMPARE(game_id_a, game_id_b);

        QVERIFY(ctx.games.count(game_id_a));
        const modeldata::Game& game = ctx.games.at(game_id_a);

        QCOMPARE(game.title, QStringLiteral("Multifile Game"));
        QCOMPARE(static_cast<int>(game.files.size()), 2);
        QCOMPARE(contains_path(file_path_a, game.files), 1);
        QCOMPARE(contains_path(file_path_b, game.files), 1);
        QCOMPARE(game.launch_cmd, common_launch);
        QCOMPARE(game.launch_workdir, common_workdir);
    }

    // Launch-only
    {
        using entry_t = decltype(ctx.games)::value_type;

        const QString title = QStringLiteral("Virtual Game");
        const auto it = std::find_if(ctx.games.cbegin(), ctx.games.cend(),
            [&title](const entry_t& entry){ return entry.second.title == title; });
        QVERIFY(it != ctx.games.cend());
    }

    // Other cases
    {
        const QString file_path = QStringLiteral(":/with_meta/horse.ext");

        QVERIFY(ctx.path_to_gameid.count(file_path));
        const size_t game_id = ctx.path_to_gameid.at(file_path);

        QVERIFY(ctx.games.count(game_id));
        const modeldata::Game& game = ctx.games.at(game_id);

        QCOMPARE(game.rating, 0.7f);
        QCOMPARE(game.launch_cmd, QStringLiteral("dummy"));
        QCOMPARE(game.launch_workdir, QStringLiteral("my/work/dir"));
        QCOMPARE(static_cast<int>(game.files.size()), 1);
        QCOMPARE(contains_path(file_path, game.files), 1);
    }
}

void test_PegasusProvider::asset_search()
{
    providers::SearchContext ctx;

    QTest::ignoreMessage(QtInfoMsg, "Collections: found `:/asset_search/metadata.txt`");
    providers::pegasus::PegasusProvider provider({QStringLiteral(":/asset_search")});
    provider.findLists(ctx);
    provider.findStaticData(ctx);

    const QString collection_name(QStringLiteral("mygames"));
    QVERIFY(ctx.collections.size() == 1);
    QVERIFY(ctx.collections.count(collection_name) == 1);
    QVERIFY(ctx.games.size() == 4);

    auto path = QStringLiteral(":/asset_search/mygame1.ext");
    QVERIFY(ctx.path_to_gameid.count(path));
    modeldata::Game* game = &ctx.games.at(ctx.path_to_gameid.at(path));
    QCOMPARE(game->assets.single(AssetType::BOX_FRONT),
        QStringLiteral("file::/asset_search/media/mygame1/box_front.png"));
    QCOMPARE(game->assets.multi(AssetType::VIDEOS),
        { QStringLiteral("file::/asset_search/media/mygame1/video.mp4") });

    path = QStringLiteral(":/asset_search/mygame3.ext");
    QVERIFY(ctx.path_to_gameid.count(path));
    game = &ctx.games.at(ctx.path_to_gameid.at(path));
    QCOMPARE(game->assets.multi(AssetType::SCREENSHOTS),
        { QStringLiteral("file::/asset_search/media/mygame3/screenshot.jpg") });
    QCOMPARE(game->assets.single(AssetType::MUSIC),
        QStringLiteral("file::/asset_search/media/mygame3/music.mp3"));

    path = QStringLiteral(":/asset_search/subdir/mygame4.ext");
    QVERIFY(ctx.path_to_gameid.count(path));
    game = &ctx.games.at(ctx.path_to_gameid.at(path));
    QCOMPARE(game->assets.single(AssetType::BACKGROUND),
        QStringLiteral("file::/asset_search/media/subdir/mygame4/background.png"));
}

void test_PegasusProvider::custom_assets()
{
    providers::SearchContext ctx;

    QTest::ignoreMessage(QtInfoMsg, "Collections: found `:/custom_assets/metadata.txt`");
    providers::pegasus::PegasusProvider provider({QStringLiteral(":/custom_assets")});
    provider.findLists(ctx);
    provider.findStaticData(ctx);

    QVERIFY(ctx.collections.size() == 1);
    QVERIFY(ctx.games.size() == 1);

    modeldata::Collection& coll = ctx.collections.begin()->second;
    QCOMPARE(coll.assets.single(AssetType::CARTRIDGE),
        QStringLiteral("file::/custom_assets/my_collection_assets/cartridge.png"));

    const auto path = QStringLiteral(":/custom_assets/mygame1.ext");
    QVERIFY(ctx.path_to_gameid.count(path));
    modeldata::Game& game = ctx.games.at(ctx.path_to_gameid.at(path));
    QCOMPARE(game.assets.single(AssetType::BOX_FRONT),
        QStringLiteral("file::/custom_assets/different_dir/whatever.png"));
}

void test_PegasusProvider::custom_directories()
{
    providers::SearchContext ctx;

    QTest::ignoreMessage(QtInfoMsg, "Collections: found `:/custom_dirs/coll/metadata.txt`");
    providers::pegasus::PegasusProvider provider({QStringLiteral(":/custom_dirs/coll")});
    provider.findLists(ctx);
    provider.findStaticData(ctx);

    QVERIFY(ctx.collections.size() == 2);
    QVERIFY(ctx.collections.count(QStringLiteral("x-files")) == 1);
    QVERIFY(ctx.collections.count(QStringLiteral("y-files")) == 1);
    QVERIFY(ctx.games.size() == 5);
    QVERIFY(ctx.collection_childs.at(QStringLiteral("x-files")).size() == 4);
    QVERIFY(ctx.collection_childs.at(QStringLiteral("y-files")).size() == 1);

    const HashMap<QString, QStringList> coll_files_map {
        { QStringLiteral("x-files"), {
            { ":/custom_dirs/coll/../a/mygame.x" },
            { ":/custom_dirs/coll/../b/mygame.x" },
            { ":/custom_dirs/c/mygame.x" },
            { ":/custom_dirs/coll/mygame.x" },
        }},
        { QStringLiteral("y-files"), {
            { ":/custom_dirs/coll/../b/mygame.y" },
        }},
    };
    verify_collected_files(ctx, coll_files_map);
}

void test_PegasusProvider::multifile()
{
    providers::SearchContext ctx;

    QTest::ignoreMessage(QtInfoMsg, "Collections: found `:/multifile/metadata.txt`");
    providers::pegasus::PegasusProvider provider({QStringLiteral(":/multifile")});
    provider.findLists(ctx);

    QCOMPARE(static_cast<int>(ctx.collections.size()),  1);
    QCOMPARE(static_cast<int>(ctx.games.size()), 2);
    QCOMPARE(static_cast<int>(ctx.games.at(0).files.size()), 1);
    QCOMPARE(static_cast<int>(ctx.games.at(1).files.size()), 2);
    QCOMPARE(static_cast<int>(ctx.path_to_gameid.size()), 3);
    QCOMPARE(static_cast<int>(ctx.collection_childs.size()), 1);

    const auto& child_vec = ctx.collection_childs.begin()->second;
    QCOMPARE(std::find(child_vec.cbegin(), child_vec.cend(), 0) != child_vec.cend(), true);
    QCOMPARE(std::find(child_vec.cbegin(), child_vec.cend(), 1) != child_vec.cend(), true);
}


QTEST_MAIN(test_PegasusProvider)
#include "test_PegasusProvider.moc"
