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

#include "model/gaming/Collection.h"
#include "model/gaming/Game.h"
#include "providers/pegasus_metadata/PegasusProvider.h"
#include "utils/HashMap.h"
#include "utils/StdHelpers.h"

#include <QString>
#include <QStringList>


namespace {

int contains_path(const QString& path, const QVector<model::GameFile*>& files)
{
    int contains = 0;
    for (const model::GameFile* const gf : files) {
        if (gf->fileinfo().filePath() == path)
            contains++;
    }
    return contains;
}

void verify_collected_files(providers::SearchContext& sctx, const HashMap<QString, QStringList>& coll_files_map)
{
    for (const auto& entry : coll_files_map) {
        const QString& coll_name = entry.first;
        const QStringList& coll_files = entry.second;

        QVector<model::Game*> expected_ptrs;

        for (const QString& abs_path : coll_files) {
            const QString can_path = QFileInfo(abs_path).canonicalFilePath();
            QVERIFY(!can_path.isEmpty());

            QVERIFY(sctx.path_to_gameid.count(can_path));
            const size_t game_id = sctx.path_to_gameid.at(can_path);

            QVERIFY(sctx.games.count(game_id));
            model::Game* game = sctx.games.at(game_id);

            QVERIFY(contains_path(abs_path, sctx.games.at(game_id)->filesConst()));
            expected_ptrs.append(game);
        }

        model::Collection& collection = *sctx.collections.at(coll_name);
        QVector<model::Game*> actual_ptrs = collection.games()->asList();

        std::sort(actual_ptrs.begin(), actual_ptrs.end());
        std::sort(expected_ptrs.begin(), expected_ptrs.end());
        QCOMPARE(actual_ptrs, actual_ptrs);
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
    void asset_search_by_title();
    void custom_assets();
    void custom_assets_multi();
    void custom_directories();
    void multifile();
    void nonASCII();
    void separate_media_dirs();
    void relative_files_only();
    void relative_files_with_dirs();
    void sort_title();
    void sort_collections();
};

void test_PegasusProvider::empty()
{
    providers::SearchContext ctx;

    QTest::ignoreMessage(QtWarningMsg, "Metafiles: No metadata file found in `:/empty`, directory ignored");
    providers::pegasus::PegasusProvider provider;
    provider.load_with_gamedirs({QStringLiteral(":/empty")});
    provider.findLists(ctx);

    QVERIFY(ctx.games.empty());
    QVERIFY(ctx.collections.empty());
}

void test_PegasusProvider::simple()
{
    providers::SearchContext ctx;

    QTest::ignoreMessage(QtInfoMsg, "Metafiles: found `:/simple/metadata.pegasus.txt`");
    providers::pegasus::PegasusProvider provider;
    provider.load_with_gamedirs({QStringLiteral(":/simple")});
    provider.findLists(ctx);

    // finds the correct collections
    QCOMPARE(static_cast<int>(ctx.collections.size()), 3);
    QVERIFY(ctx.collections.count(QStringLiteral("My Games")));
    QVERIFY(ctx.collections.count(QStringLiteral("Favorite games")));
    QVERIFY(ctx.collections.count(QStringLiteral("Multi-game ROMs")));

    // finds the correct amount of games
    QCOMPARE(static_cast<int>(ctx.games.size()), 8);
    QVERIFY(ctx.collections.at(QStringLiteral("My Games"))->gamesConst().size() == 8);
    QVERIFY(ctx.collections.at(QStringLiteral("Favorite games"))->gamesConst().size() == 3);
    QVERIFY(ctx.collections.at(QStringLiteral("Multi-game ROMs"))->gamesConst().size() == 1);

    // finds the correct files for the collections
    const HashMap<QString, QStringList> coll_files_map {
        { QStringLiteral("My Games"), {
            { ":/simple/mygame1.ext" },
            { ":/simple/mygame2.EXT" },
            { ":/simple/mygame3.ext" },
            { ":/simple/favgame1.ext" },
            { ":/simple/favgame2.ext" },
            { ":/simple/game with spaces.ext" },
            { ":/simple/9999-in-1.ext" },
            { ":/simple/subdir/game_in_subdir.EXT" },
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

    QTest::ignoreMessage(QtInfoMsg, "Metafiles: found `:/with_meta/metadata.pegasus.txt`");
    QTest::ignoreMessage(QtWarningMsg, "Metafiles: `:/with_meta/metadata.pegasus.txt`, line 60: duplicate file `horse.ext`");
    QTest::ignoreMessage(QtWarningMsg, "Metafiles: `:/with_meta/metadata.pegasus.txt`, line 63: failed to parse rating value");
    QTest::ignoreMessage(QtWarningMsg, "Metafiles: `:/with_meta/metadata.pegasus.txt`, line 65: incorrect date format, should be YYYY, YYYY-MM or YYYY-MM-DD");
    QTest::ignoreMessage(QtWarningMsg, "Metafiles: `:/with_meta/metadata.pegasus.txt`, line 66: incorrect date format, should be YYYY, YYYY-MM or YYYY-MM-DD");
    QTest::ignoreMessage(QtWarningMsg, "Metafiles: `:/with_meta/metadata.pegasus.txt`, line 68: duplicate file `horse.ext`");
    QTest::ignoreMessage(QtWarningMsg, "Metafiles: `:/with_meta/metadata.pegasus.txt`, line 69: unrecognized game property `asd`, ignored");

    providers::pegasus::PegasusProvider provider;
    provider.load_with_gamedirs({QStringLiteral(":/with_meta")});
    provider.findLists(ctx);

    QCOMPARE(static_cast<int>(ctx.collections.size()), 1);
    QCOMPARE(static_cast<int>(ctx.games.size()), 6);

    const auto common_launch = QStringLiteral("launcher.exe '{file.path}'");
    const auto common_workdir = QStringLiteral("some/workdir");
    const auto common_basedir = QStringLiteral(":/with_meta");

    // Collection
    {
        const QString collection_name(QStringLiteral("My Games"));

        QVERIFY(ctx.collections.count(collection_name));
        const model::Collection& coll = *ctx.collections.at(collection_name);

        QCOMPARE(coll.shortName(), QStringLiteral("mygames"));
        QCOMPARE(coll.summary(), QStringLiteral("this is the summary"));
        QCOMPARE(coll.description(), QStringLiteral("this is the description"));
        QCOMPARE(coll.commonLaunchCmd(), common_launch);
        QCOMPARE(coll.commonLaunchWorkdir(), common_workdir);
        QCOMPARE(coll.commonLaunchCmdBasedir(), common_basedir);
    }

    // Game before the first collection entry
    {
        const QString file_path = QStringLiteral(":/with_meta/pre.ext");

        QVERIFY(ctx.path_to_gameid.count(file_path));
        const size_t game_id = ctx.path_to_gameid.at(file_path);

        QVERIFY(ctx.games.count(game_id));
        const model::Game& game = *ctx.games.at(game_id);

        QCOMPARE(game.launchCmd(), common_launch);
        QCOMPARE(game.launchWorkdir(), common_workdir);
        QCOMPARE(game.launchCmdBasedir(), common_basedir);
    }

    // Basic
    {
        const QString file_path = QStringLiteral(":/with_meta/basic.ext");

        QVERIFY(ctx.path_to_gameid.count(file_path));
        const size_t game_id = ctx.path_to_gameid.at(file_path);

        QVERIFY(ctx.games.count(game_id));
        const model::Game& game = *ctx.games.at(game_id);

        QCOMPARE(game.title(), QStringLiteral("A simple game"));
        QCOMPARE(game.developerListConst(), QStringList({"Dev", "Dev with Spaces"}));
        QCOMPARE(game.publisherListConst(), QStringList({"The Company"}));
        QCOMPARE(game.genreListConst(), QStringList({"genre1", "genre2", "genre with spaces"}));
        QCOMPARE(game.playerCount(), 4);
        QCOMPARE(game.releaseDate(), QDate(1998, 5, 1));
        QCOMPARE(game.summary(), QStringLiteral("something short here"));
        QCOMPARE(game.description(), QStringLiteral("a very long\n\ndescription"));
        QCOMPARE(game.rating(), 0.8f);
        QCOMPARE(game.launchCmd(), common_launch);
        QCOMPARE(game.launchWorkdir(), common_workdir);
        QCOMPARE(game.launchCmdBasedir(), common_basedir);
        QCOMPARE(static_cast<int>(game.filesConst().size()), 1);
        QCOMPARE(contains_path(file_path, game.filesConst()), 1);
    }

    // Subdir
    {
        const QString file_path = QStringLiteral(":/with_meta/subdir/game_in_subdir.ext");

        QVERIFY(ctx.path_to_gameid.count(file_path));
        const size_t game_id = ctx.path_to_gameid.at(file_path);

        QVERIFY(ctx.games.count(game_id));
        const model::Game& game = *ctx.games.at(game_id);

        QCOMPARE(game.title(), QStringLiteral("Subdir Game"));
        QCOMPARE(game.launchCmd(), common_launch);
        QCOMPARE(game.launchWorkdir(), common_workdir);
        QCOMPARE(game.launchCmdBasedir(), common_basedir);
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
        const model::Game& game = *ctx.games.at(game_id_a);

        QCOMPARE(game.title(), QStringLiteral("Multifile Game"));
        QCOMPARE(static_cast<int>(game.filesConst().size()), 2);
        QCOMPARE(contains_path(file_path_a, game.filesConst()), 1);
        QCOMPARE(contains_path(file_path_b, game.filesConst()), 1);
        QCOMPARE(game.launchCmd(), common_launch);
        QCOMPARE(game.launchWorkdir(), common_workdir);
        QCOMPARE(game.launchCmdBasedir(), common_basedir);
    }

    // Launch-only
    {
        using entry_t = decltype(ctx.games)::value_type;

        const QString title = QStringLiteral("Virtual Game");
        const auto it = std::find_if(ctx.games.cbegin(), ctx.games.cend(),
            [&title](const entry_t& entry){ return entry.second->title() == title; });
        QVERIFY(it != ctx.games.cend());
    }

    // Other cases
    {
        const QString file_path = QStringLiteral(":/with_meta/horse.ext");

        QVERIFY(ctx.path_to_gameid.count(file_path));
        const size_t game_id = ctx.path_to_gameid.at(file_path);

        QVERIFY(ctx.games.count(game_id));
        const model::Game& game = *ctx.games.at(game_id);

        QCOMPARE(game.rating(), 0.7f);
        QCOMPARE(game.launchCmd(), QStringLiteral("dummy"));
        QCOMPARE(game.launchWorkdir(), QStringLiteral("some\\nice/work/dir"));
        QCOMPARE(game.launchCmdBasedir(), common_basedir);
        QCOMPARE(static_cast<int>(game.filesConst().size()), 1);
        QCOMPARE(contains_path(file_path, game.filesConst()), 1);
        QCOMPARE(game.summary(), QStringLiteral("manual break\nhere"));
        QCOMPARE(game.description(), QStringLiteral("manual break\nhere\n\nline end\n here\n\nescaped\\nbreak"));
    }
}

void test_PegasusProvider::asset_search()
{
    providers::SearchContext ctx;

    QTest::ignoreMessage(QtInfoMsg, "Metafiles: found `:/asset_search/metadata.txt`");
    providers::pegasus::PegasusProvider provider;
    provider.load_with_gamedirs({QStringLiteral(":/asset_search")});
    provider.findLists(ctx);
    provider.findStaticData(ctx);

    const QString collection_name(QStringLiteral("mygames"));
    QVERIFY(ctx.collections.size() == 1);
    QVERIFY(ctx.collections.count(collection_name) == 1);
    QVERIFY(ctx.games.size() == 4);
    QVERIFY(VEC_CONTAINS(ctx.game_root_dirs, QStringLiteral(":/asset_search")));

    auto path = QStringLiteral(":/asset_search/mygame1.ext");
    QVERIFY(ctx.path_to_gameid.count(path));
    model::Game* game = ctx.games.at(ctx.path_to_gameid.at(path));
    QCOMPARE(game->assets().boxFront(), QStringLiteral("file::/asset_search/media/mygame1/box_front.png"));
    qDebug() << game->assets().videoList();
    QCOMPARE(game->assets().videoList(), { QStringLiteral("file::/asset_search/media/mygame1/video.mp4") });

    path = QStringLiteral(":/asset_search/mygame3.ext");
    QVERIFY(ctx.path_to_gameid.count(path));
    game = ctx.games.at(ctx.path_to_gameid.at(path));
    QCOMPARE(game->assets().screenshotList(),
        { QStringLiteral("file::/asset_search/media/mygame3/screenshot.jpg") });
    QCOMPARE(game->assets().music(),
        QStringLiteral("file::/asset_search/media/mygame3/music.mp3"));

    path = QStringLiteral(":/asset_search/subdir/mygame4.ext");
    QVERIFY(ctx.path_to_gameid.count(path));
    game = ctx.games.at(ctx.path_to_gameid.at(path));
    QCOMPARE(game->assets().background(),
        QStringLiteral("file::/asset_search/media/subdir/mygame4/background.png"));
}

void test_PegasusProvider::asset_search_by_title()
{
    providers::SearchContext ctx;

    QTest::ignoreMessage(QtInfoMsg, "Metafiles: found `:/asset_search_by_title/metadata.txt`");
    providers::pegasus::PegasusProvider provider;
    provider.load_with_gamedirs({QStringLiteral(":/asset_search_by_title")});
    provider.findLists(ctx);
    provider.findStaticData(ctx);

    const QString collection_name(QStringLiteral("mygames"));
    QVERIFY(ctx.collections.size() == 1);
    QVERIFY(ctx.collections.count(collection_name) == 1);
    QVERIFY(ctx.games.size() == 1);
    QVERIFY(VEC_CONTAINS(ctx.game_root_dirs, QStringLiteral(":/asset_search_by_title")));

    const QString path = QStringLiteral(":/asset_search_by_title/mygame.ext");
    QVERIFY(ctx.path_to_gameid.count(path));
    model::Game* const game = ctx.games.at(ctx.path_to_gameid.at(path));
    QCOMPARE(game->assets().boxFront(),
        QStringLiteral("file::/asset_search_by_title/media/My Precious Game/box_front.png"));
    QCOMPARE(game->assets().videoList(),
        { QStringLiteral("file::/asset_search_by_title/media/My Precious Game/video.mp4") });
}

void test_PegasusProvider::custom_assets()
{
    providers::SearchContext ctx;

    QTest::ignoreMessage(QtInfoMsg, "Metafiles: found `:/custom_assets/metadata.txt`");
    providers::pegasus::PegasusProvider provider;
    provider.load_with_gamedirs({QStringLiteral(":/custom_assets")});
    provider.findLists(ctx);
    provider.findStaticData(ctx);

    QVERIFY(ctx.collections.size() == 1);
    QVERIFY(ctx.games.size() == 1);

    model::Collection& coll = *ctx.collections.begin()->second;
    QCOMPARE(coll.assets().cartridge(),
        QStringLiteral("file::/custom_assets/my_collection_assets/cartridge.png"));

    const auto path = QStringLiteral(":/custom_assets/mygame1.ext");
    QVERIFY(ctx.path_to_gameid.count(path));
    model::Game& game = *ctx.games.at(ctx.path_to_gameid.at(path));
    QCOMPARE(game.assets().boxFront(),
        QStringLiteral("file::/custom_assets/different_dir/whatever.png"));
}

void test_PegasusProvider::custom_assets_multi()
{
    providers::SearchContext ctx;

    QTest::ignoreMessage(QtInfoMsg, "Metafiles: found `:/custom_assets_multi/metadata.txt`");
    providers::pegasus::PegasusProvider provider;
    provider.load_with_gamedirs({QStringLiteral(":/custom_assets_multi")});
    provider.findLists(ctx);
    provider.findStaticData(ctx);

    QVERIFY(ctx.collections.size() == 1);
    QVERIFY(ctx.games.size() == 2);

    {
        const auto path = QStringLiteral(":/custom_assets_multi/game1.ext");
        QVERIFY(ctx.path_to_gameid.count(path));
        model::Game& game = *ctx.games.at(ctx.path_to_gameid.at(path));
        QCOMPARE(game.assets().videoList(),
            QStringList(QStringLiteral("file::/custom_assets_multi/videos/a.mp4")));
    }{
        const auto path = QStringLiteral(":/custom_assets_multi/game2.ext");
        QVERIFY(ctx.path_to_gameid.count(path));
        model::Game& game = *ctx.games.at(ctx.path_to_gameid.at(path));
        QCOMPARE(game.assets().videoList(), QStringList({
            QStringLiteral("file::/custom_assets_multi/videos/a.mp4"),
            QStringLiteral("file::/custom_assets_multi/videos/b.mp4"),
            QStringLiteral("file::/custom_assets_multi/videos/c.mp4"),
        }));
    }
}

void test_PegasusProvider::custom_directories()
{
    providers::SearchContext ctx;

    QTest::ignoreMessage(QtInfoMsg, "Metafiles: found `:/custom_dirs/coll/metadata.txt`");
    providers::pegasus::PegasusProvider provider;
    provider.load_with_gamedirs({QStringLiteral(":/custom_dirs/coll")});
    provider.findLists(ctx);
    provider.findStaticData(ctx);

    QVERIFY(ctx.collections.size() == 2);
    QVERIFY(ctx.collections.count(QStringLiteral("x-files")) == 1);
    QVERIFY(ctx.collections.count(QStringLiteral("y-files")) == 1);
    QVERIFY(ctx.games.size() == 5);
    QVERIFY(ctx.collections.at(QStringLiteral("x-files"))->gamesConst().size() == 4);
    QVERIFY(ctx.collections.at(QStringLiteral("y-files"))->gamesConst().size() == 1);

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

    QTest::ignoreMessage(QtInfoMsg, "Metafiles: found `:/multifile/metadata.txt`");
    providers::pegasus::PegasusProvider provider;
    provider.load_with_gamedirs({QStringLiteral(":/multifile")});
    provider.findLists(ctx);

    QCOMPARE(static_cast<int>(ctx.collections.size()),  1);
    QCOMPARE(static_cast<int>(ctx.games.size()), 2);
    QCOMPARE(static_cast<int>(ctx.games.at(0)->files()->size()), 1);
    QCOMPARE(static_cast<int>(ctx.games.at(1)->files()->size()), 2);
    QCOMPARE(static_cast<int>(ctx.path_to_gameid.size()), 3);

    const QVector<model::Game*>& child_vec = ctx.collections.cbegin()->second->gamesConst();
    QCOMPARE(std::find(child_vec.cbegin(), child_vec.cend(), ctx.games.at(0)) != child_vec.cend(), true);
    QCOMPARE(std::find(child_vec.cbegin(), child_vec.cend(), ctx.games.at(1)) != child_vec.cend(), true);
}

void test_PegasusProvider::nonASCII()
{
    struct TestEntry {
        QString title;
        QString filename;
        QString desc;
    };
    // It's 2019 and yet some compilers still have Unicode troubles...
    const std::vector<TestEntry> entries {
        { "AsciiGame", "ascii.ext", "A simple ASCII filename" },
        {
            u8"\u00C1rv\u00EDzt\u0171r\u0151", // Árvíztűrő
            u8"\u00E1rv\u00EDzt\u0171r\u0151.ext", // árvíztűrő.ext
            u8"\u00C1rv\u00EDzt\u0171r\u0151 t\u00FCk\u00F6rf\u00FAr\u00F3g\u00E9p", // Árvíztűrő tükörfúrógép
        },
        {
            u8"\u65E5\u672C\u30B2\u30FC\u30E0", // 日本ゲーム
            u8"\u30B2\u30FC\u30E0.ext", // ゲーム.ext
            u8"\u8272\u306F\u5302\u3078\u3069 \u6563\u308A\u306C\u308B\u3092", // 色は匂へど 散りぬるを
        },
    };
    QCOMPARE(entries.at(1).title.size(), 9); // Árvíztűrő, spec = 2 bytes
    QCOMPARE(entries.at(1).title.toUtf8().size(), 13);
    QCOMPARE(entries.at(2).title.size(), 5); // 日本ゲーム, spec = 3 bytes
    QCOMPARE(entries.at(2).title.toUtf8().size(), 15);

    QTemporaryDir tempdir;
    QVERIFY(tempdir.isValid());
    {

        const QString metapath = tempdir.filePath("metadata.txt");
        QFile metafile(metapath);
        QVERIFY(metafile.open(QFile::WriteOnly | QFile::Text));
        QTextStream stream(&metafile);
        stream.setCodec("UTF-8");
        stream << QStringLiteral("collection: Test\nextension: ext\n\n");

        for (const TestEntry& entry : entries) {
            stream
                << "\ngame: " << entry.title
                << "\nfile: " << entry.filename
                << "\ndescription: " << entry.desc
                << "\n";
            {
                const QString filepath = tempdir.filePath(entry.filename);
                QFile file(filepath);
                QVERIFY(file.open(QFile::WriteOnly));
                QTextStream(&file) << QChar('\n');
            }
            {
                const QString mediapath = "media/" + entry.title;
                QVERIFY(QDir(tempdir.path()).mkpath(mediapath));
                const QString filepath = tempdir.filePath(mediapath + "/box_front.png");
                QFile file(filepath);
                QVERIFY(file.open(QFile::WriteOnly));
                QTextStream(&file) << QChar('\n');
            }
        }
    }

    providers::SearchContext ctx;

    const QString ignored_msg = "Metafiles: found `" + tempdir.path() + "/metadata.txt`";
    QTest::ignoreMessage(QtInfoMsg, ignored_msg.toLocal8Bit());
    providers::pegasus::PegasusProvider provider;
    provider.load_with_gamedirs({tempdir.path()});
    provider.findLists(ctx);
    provider.findStaticData(ctx);

    QCOMPARE(static_cast<int>(ctx.collections.size()),  1);
    QCOMPARE(ctx.games.size(), entries.size());

    for (const TestEntry& expected : entries) {
        auto it = std::find_if(ctx.games.begin(), ctx.games.end(),
            [&expected](const decltype(ctx.games)::value_type& pair){ return pair.second->title() == expected.title; });
        QVERIFY(it != ctx.games.cend());

        model::Game& game = *it->second;
        QCOMPARE(game.title(), expected.title);
        QCOMPARE(static_cast<int>(game.filesConst().size()), 1);
        QCOMPARE(game.filesConst().front()->fileinfo().absoluteFilePath(), tempdir.path() + "/" + expected.filename);
        QCOMPARE(game.filesConst().front()->fileinfo().exists(), true);
        QCOMPARE(game.description(), expected.desc);

        // FIXME: This fails on macOS,
        // because the files are created under /var/.../<appname>/
        // but the media searching happens in /private/var/.../<appname>/
        // for some reason
#ifndef Q_OS_DARWIN
        const QString expected_asset_path = QUrl::fromLocalFile(tempdir.path() + "/media/" + expected.title + "/box_front.png").toString();
        QCOMPARE(game.assets().boxFront(), expected_asset_path);
#endif
    }
}

void test_PegasusProvider::separate_media_dirs()
{
    // NOTE: see issue 407

    providers::SearchContext ctx;
    QTest::ignoreMessage(QtInfoMsg, "Metafiles: found `:/separate_media_dirs/metadata/metadata.pegasus.txt`");
    providers::pegasus::PegasusProvider provider;
    provider.load_with_gamedirs({QStringLiteral(":/separate_media_dirs/metadata")});

    provider.findLists(ctx);
    QCOMPARE(static_cast<int>(ctx.collections.size()),  2);
    QCOMPARE(static_cast<int>(ctx.games.size()), 2);
    QVERIFY(VEC_CONTAINS(ctx.game_root_dirs, QStringLiteral(":/separate_media_dirs/metadata")));
    QVERIFY(VEC_CONTAINS(ctx.game_root_dirs, QStringLiteral(":/separate_media_dirs/metadata/../games-a")));
    QVERIFY(VEC_CONTAINS(ctx.game_root_dirs, QStringLiteral(":/separate_media_dirs/metadata/../games-b")));

    provider.findStaticData(ctx);
    // NOTE: Yes, canonicalPath() returns a path with '..' in it...
    QCOMPARE(ctx.games.at(0)->assets().boxFront(), QStringLiteral("file::/separate_media_dirs/metadata/../games-a/media/Game 1/box_front.png"));
    QCOMPARE(ctx.games.at(1)->assets().boxFront(), QStringLiteral("file::/separate_media_dirs/metadata/../games-b/media/Game 2/box_front.png"));
}

void test_PegasusProvider::relative_files_only()
{
    providers::SearchContext ctx;
    QTest::ignoreMessage(QtInfoMsg, "Metafiles: found `:/relative_files/coll/metadata.txt`");
    providers::pegasus::PegasusProvider provider;
    provider.load_with_gamedirs({QStringLiteral(":/relative_files/coll")});
    provider.findLists(ctx);

    QVERIFY(ctx.collections.size() == 1);
    QVERIFY(ctx.collections.count(QStringLiteral("myfiles")) == 1);

    const HashMap<QString, QStringList> coll_files_map {
        { QStringLiteral("myfiles"), {
            { ":/relative_files/coll/../a/game_a.ext" },
            { ":/relative_files/coll/game_here.ext" },
            { ":/relative_files/coll/b/game_b.ext" },
            { ":/relative_files/coll/./c/game_c.ext" },
        }},
    };
    verify_collected_files(ctx, coll_files_map);
}

void test_PegasusProvider::relative_files_with_dirs()
{
    providers::SearchContext ctx;
    QTest::ignoreMessage(QtInfoMsg, "Metafiles: found `:/relative_files_with_dirs/coll/metadata.txt`");
    providers::pegasus::PegasusProvider provider;
    provider.load_with_gamedirs({QStringLiteral(":/relative_files_with_dirs/coll")});
    provider.findLists(ctx);

    QVERIFY(ctx.collections.size() == 1);
    QVERIFY(ctx.collections.count(QStringLiteral("myfiles")) == 1);

    const HashMap<QString, QStringList> coll_files_map {
        { QStringLiteral("myfiles"), {
            { ":/relative_files_with_dirs/coll/../a/game_a.x" },
            { ":/relative_files_with_dirs/coll/../b/game_b.x" },
            { ":/relative_files_with_dirs/coll/game_here.x" },
        }},
    };
    verify_collected_files(ctx, coll_files_map);
}

void test_PegasusProvider::sort_title()
{
    providers::SearchContext ctx;
    QTest::ignoreMessage(QtInfoMsg, "Metafiles: found `:/sort_title/metadata.txt`");
    providers::pegasus::PegasusProvider provider;
    provider.load_with_gamedirs({QStringLiteral(":/sort_title")});
    provider.findLists(ctx);

    QVERIFY(ctx.games.size() == 4);
    QVERIFY(ctx.collections.size() == 1);
    QVERIFY(ctx.collections.count(QStringLiteral("test")) == 1);

    std::vector<model::Game*> games;
    games.reserve(ctx.games.size());
    for (auto& keyval : ctx.games) {
        qDebug() << keyval.second << keyval.second->sortTitle() << keyval.second->title();
        games.emplace_back(keyval.second);
    }

    std::sort(games.begin(), games.end(),
        [](const model::Game* const a, const model::Game* const b) {
            return QString::localeAwareCompare(a->sortTitle(), b->sortTitle()) < 0;
        }
    );

    QCOMPARE(games.at(0)->title(), QStringLiteral("Game I"));
    QCOMPARE(games.at(1)->title(), QStringLiteral("Game IV"));
    QCOMPARE(games.at(2)->title(), QStringLiteral("Game 8"));
    QCOMPARE(games.at(3)->title(), QStringLiteral("Game IX"));
}

void test_PegasusProvider::sort_collections()
{
    providers::SearchContext ctx;
    QTest::ignoreMessage(QtInfoMsg, "Metafiles: found `:/sort_collections/metadata.pegasus.txt`");
    providers::pegasus::PegasusProvider provider;
    provider.load_with_gamedirs({QStringLiteral(":/sort_collections")});
    provider.findLists(ctx);

    QVERIFY(ctx.games.size() == 1);
    QVERIFY(ctx.collections.size() == 4);

    std::vector<model::Collection*> collections;
    collections.reserve(ctx.collections.size());
    for (auto& keyval : ctx.collections)
        collections.emplace_back(keyval.second);

    std::sort(collections.begin(), collections.end(), model::sort_collections);

    QCOMPARE(collections.at(0)->name(), QStringLiteral("Collection I"));
    QCOMPARE(collections.at(1)->name(), QStringLiteral("Collection IV"));
    QCOMPARE(collections.at(2)->name(), QStringLiteral("Collection 8"));
    QCOMPARE(collections.at(3)->name(), QStringLiteral("Collection IX"));
}


QTEST_MAIN(test_PegasusProvider)
#include "test_PegasusProvider.moc"
