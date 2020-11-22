// Pegasus Frontend
// Copyright (C) 2017-2020  Mátyás Mustoha
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

#include "Log.h"
#include "model/gaming/Assets.h"
#include "model/gaming/Collection.h"
#include "model/gaming/Game.h"
#include "model/gaming/GameFile.h"
#include "providers/SearchContext.h"
#include "providers/pegasus_metadata/PegasusProvider.h"
#include "utils/HashMap.h"
#include "utils/StdHelpers.h"

#include <QString>
#include <QStringList>


namespace {

#define PATHMSG(msg, path) qUtf8Printable(QStringLiteral(msg).arg(QDir::toNativeSeparators(QStringLiteral(path))))

const model::Collection* get_collection_ptr(const QVector<model::Collection*>& list, const QString& name)
{
    const auto it = std::find_if(list.cbegin(), list.cend(),
        [&name](const model::Collection* const ptr){ return ptr->name() == name; });
    return it != list.cend()
        ? *it
        : nullptr;
}

const model::Collection& get_collection(const QVector<model::Collection*>& list, const QString& name)
{
    const model::Collection* ptr = get_collection_ptr(list, name);
    Q_ASSERT(ptr != nullptr);
    return *ptr;
}

bool has_collection(const QVector<model::Collection*>& list, const QString& name)
{
    return get_collection_ptr(list, name) != nullptr;
}


const model::Game* get_game_ptr_by_file_path(const QVector<model::Game*>& list, const QString& path)
{
    const auto it = std::find_if(
        list.cbegin(),
        list.cend(),
        [&path](const model::Game* const game){ return std::any_of(
            game->filesConst().cbegin(),
            game->filesConst().cend(),
            [&path](const model::GameFile* gf){ return gf->fileinfo().canonicalFilePath() == path; });
        });
    return it != list.cend()
        ? *it
        : nullptr;
}

const model::Game& get_game_by_file_path(const QVector<model::Game*>& list, const QString& path)
{
    const model::Game* ptr = get_game_ptr_by_file_path(list, path);
    Q_ASSERT(ptr != nullptr);
    return *ptr;
}

bool has_game_file(const QVector<model::Game*>& list, const QString& path)
{
    return get_game_ptr_by_file_path(list, path) != nullptr;
}

void verify_collected_files(
    const QVector<model::Collection*>& collections,
    const HashMap<QString, QStringList>& expected_collection_files)
{
    for (const auto& [collname, expected_files] : expected_collection_files) {
        QVERIFY(has_collection(collections, collname));
        const model::Collection& coll = get_collection(collections, collname);

        for (const QString& abs_path : expected_files) {
            const QString can_path = QFileInfo(abs_path).canonicalFilePath();
            Q_ASSERT(!can_path.isEmpty());

            QVERIFY(has_game_file(coll.gamesConst(), can_path));
        }
    }
}

} // namespace


class test_PegasusProvider : public QObject {
    Q_OBJECT

private slots:
    void initTestCase() {
        Log::init_qttest();
    }

    void empty();
    void simple();
    void with_meta();
    void custom_assets();
    void custom_assets_multi();
    void custom_directories();
    void multifile();
    void multicoll();
    void nonASCII();
    void relative_files_only();
    void relative_files_with_dirs();
};

void test_PegasusProvider::empty()
{
    QTest::ignoreMessage(QtInfoMsg, "Metafiles: No metadata files found");

    providers::SearchContext sctx({ QStringLiteral(":/empty") });
    providers::pegasus::PegasusProvider().run(sctx);
    const auto [collections, games] = sctx.finalize(this);

    QVERIFY(games.isEmpty());
    QVERIFY(collections.isEmpty());
}

void test_PegasusProvider::simple()
{
    QTest::ignoreMessage(QtInfoMsg, PATHMSG("Metafiles: Found `%1`", ":/simple/metadata.pegasus.txt"));

    providers::SearchContext sctx({QStringLiteral(":/simple")});
    providers::pegasus::PegasusProvider().run(sctx);
    const auto [collections, games] = sctx.finalize(this);

    // finds the correct collections
    QCOMPARE(collections.size(), 3);
    QVERIFY(has_collection(collections, QStringLiteral("My Games")));
    QVERIFY(has_collection(collections, QStringLiteral("Favorite games")));
    QVERIFY(has_collection(collections, QStringLiteral("Multi-game ROMs")));

    // finds the correct amount of games
    QCOMPARE(games.size(), 8);
    QCOMPARE(get_collection(collections, QStringLiteral("My Games")).gamesConst().size(), 8);
    QCOMPARE(get_collection(collections, QStringLiteral("Favorite games")).gamesConst().size(), 3);
    QCOMPARE(get_collection(collections, QStringLiteral("Multi-game ROMs")).gamesConst().size(), 1);

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

    verify_collected_files(collections, coll_files_map);
}

void test_PegasusProvider::with_meta()
{
    QTest::ignoreMessage(QtInfoMsg, PATHMSG("Metafiles: Found `%1`", ":/with_meta/metadata.pegasus.txt"));
    QTest::ignoreMessage(QtWarningMsg, PATHMSG("Metafiles: `%1`, line 62: Duplicate file entry detected: `horse.ext`", ":/with_meta/metadata.pegasus.txt"));
    QTest::ignoreMessage(QtWarningMsg, PATHMSG("Metafiles: `%1`, line 65: Failed to parse the rating value", ":/with_meta/metadata.pegasus.txt"));
    QTest::ignoreMessage(QtWarningMsg, PATHMSG("Metafiles: `%1`, line 67: Incorrect date format, should be YYYY, YYYY-MM or YYYY-MM-DD", ":/with_meta/metadata.pegasus.txt"));
    QTest::ignoreMessage(QtWarningMsg, PATHMSG("Metafiles: `%1`, line 68: Incorrect date format, should be YYYY, YYYY-MM or YYYY-MM-DD", ":/with_meta/metadata.pegasus.txt"));
    QTest::ignoreMessage(QtWarningMsg, PATHMSG("Metafiles: `%1`, line 70: Duplicate file entry detected: `horse.ext`", ":/with_meta/metadata.pegasus.txt"));
    QTest::ignoreMessage(QtWarningMsg, PATHMSG("Metafiles: `%1`, line 71: Unrecognized game property `asd`, ignored", ":/with_meta/metadata.pegasus.txt"));
    QTest::ignoreMessage(QtWarningMsg, "The game 'Virtual Game' does not belong to any collections, ignored");

    providers::SearchContext sctx({QStringLiteral(":/with_meta")});
    providers::pegasus::PegasusProvider().run(sctx);
    const auto [collections, games] = sctx.finalize(this);

    QCOMPARE(collections.size(), 1);
    QCOMPARE(games.size(), 5);

    const auto common_launch = QStringLiteral("launcher.exe '{file.path}'");
    const auto common_workdir = QStringLiteral("some/workdir");
    const auto common_basedir = QStringLiteral(":/with_meta");

    // Collection
    {
        const auto collection_name = QStringLiteral("My Games");

        QVERIFY(has_collection(collections, collection_name));
        const model::Collection& coll = get_collection(collections, collection_name);

        QCOMPARE(coll.shortName(), QStringLiteral("mygames"));
        QCOMPARE(coll.sortBy(), QStringLiteral("Collection IV"));
        QCOMPARE(coll.summary(), QStringLiteral("this is the summary"));
        QCOMPARE(coll.description(), QStringLiteral("this is the description"));
        QCOMPARE(coll.commonLaunchCmd(), common_launch);
        QCOMPARE(coll.commonLaunchWorkdir(), common_workdir);
        QCOMPARE(coll.commonLaunchCmdBasedir(), common_basedir);
    }

    // Game before the first collection entry
    {
        const auto file_path = QStringLiteral(":/with_meta/pre.ext");

        QVERIFY(has_game_file(games, file_path));
        const model::Game& game = get_game_by_file_path(games, file_path);

        QCOMPARE(game.launchCmd(), common_launch);
        QCOMPARE(game.launchWorkdir(), common_workdir);
        QCOMPARE(game.launchCmdBasedir(), common_basedir);
    }

    // Basic
    {
        const auto file_path = QStringLiteral(":/with_meta/basic.ext");

        QVERIFY(has_game_file(games, file_path));
        const model::Game& game = get_game_by_file_path(games, file_path);

        QCOMPARE(game.title(), QStringLiteral("A simple game"));
        QCOMPARE(game.sortBy(), QStringLiteral("Game IX"));
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
        QCOMPARE(game.filesConst().size(), 1);
    }

    // Subdir
    {
        const QString file_path = QStringLiteral(":/with_meta/subdir/game_in_subdir.ext");

        QVERIFY(has_game_file(games, file_path));
        const model::Game& game = get_game_by_file_path(games, file_path);

        QCOMPARE(game.title(), QStringLiteral("Subdir Game"));
        QCOMPARE(game.launchCmd(), common_launch);
        QCOMPARE(game.launchWorkdir(), common_workdir);
        QCOMPARE(game.launchCmdBasedir(), common_basedir);
    }

    // Multifile
    {
        const QString file_path_a = QStringLiteral(":/with_meta/multi.a.ext");
        const QString file_path_b = QStringLiteral(":/with_meta/multi.b.ext");

        QVERIFY(has_game_file(games, file_path_a));
        QVERIFY(has_game_file(games, file_path_b));
        QCOMPARE(get_game_ptr_by_file_path(games, file_path_a), get_game_ptr_by_file_path(games, file_path_b));

        const model::Game& game = get_game_by_file_path(games, file_path_a);

        QCOMPARE(game.title(), QStringLiteral("Multifile Game"));
        QCOMPARE(game.filesConst().size(), 2);
        QCOMPARE(game.launchCmd(), common_launch);
        QCOMPARE(game.launchWorkdir(), common_workdir);
        QCOMPARE(game.launchCmdBasedir(), common_basedir);
    }

    // Launch-only
    // TODO: add non-file entry support
    /*{
        using entry_t = std::remove_reference<decltype(ctx.games())>::type::value_type;

        const QString title = QStringLiteral("Virtual Game");
        const auto it = std::find_if(ctx.games().cbegin(), ctx.games().cend(),
            [&title](const entry_t& entry){ return entry.second.inner().title() == title; });
        QVERIFY(it != ctx.games().cend());
    }*/

    // Other cases
    {
        const QString file_path = QStringLiteral(":/with_meta/horse.ext");

        QVERIFY(has_game_file(games, file_path));
        const model::Game& game = get_game_by_file_path(games, file_path);

        QCOMPARE(game.rating(), 0.7f);
        QCOMPARE(game.launchCmd(), QStringLiteral("dummy"));
        QCOMPARE(game.launchWorkdir(), QStringLiteral("some\\nice/work/dir"));
        QCOMPARE(game.launchCmdBasedir(), common_basedir);
        QCOMPARE(game.filesConst().size(), 1);
        QCOMPARE(game.summary(), QStringLiteral("manual break\nhere"));
        QCOMPARE(game.description(), QStringLiteral("manual break\nhere\n\nline end\n here\n\nescaped\\nbreak"));
    }
}

void test_PegasusProvider::custom_assets()
{
    QTest::ignoreMessage(QtInfoMsg, PATHMSG("Metafiles: Found `%1`", ":/custom_assets/metadata.txt"));

    providers::SearchContext sctx({QStringLiteral(":/custom_assets")});
    providers::pegasus::PegasusProvider().run(sctx);
    const auto [collections, games] = sctx.finalize(this);

    QCOMPARE(collections.size(), 1);
    const model::Collection& coll = *collections.first();
    QCOMPARE(coll.assets().cartridge(), QStringLiteral("file::/custom_assets/my_collection_assets/cartridge.png"));

    QCOMPARE(games.size(), 1);
    const model::Game& game = *games.first();
    QCOMPARE(game.assets().boxFront(), QStringLiteral("file::/custom_assets/different_dir/whatever.png"));
}

void test_PegasusProvider::custom_assets_multi()
{
    QTest::ignoreMessage(QtInfoMsg, PATHMSG("Metafiles: Found `%1`", ":/custom_assets_multi/metadata.txt"));

    providers::SearchContext sctx({QStringLiteral(":/custom_assets_multi")});
    providers::pegasus::PegasusProvider().run(sctx);
    const auto [collections, games] = sctx.finalize(this);

    QCOMPARE(collections.size(), 1);
    QCOMPARE(games.size(), 2);

    {
        const auto path = QStringLiteral(":/custom_assets_multi/game1.ext");
        QVERIFY(has_game_file(games, path));
        const model::Game& game = get_game_by_file_path(games, path);
        QCOMPARE(game.assets().videoList(),
            QStringList(QStringLiteral("file::/custom_assets_multi/videos/a.mp4")));
    }
    {
        const auto path = QStringLiteral(":/custom_assets_multi/game2.ext");
        QVERIFY(has_game_file(games, path));
        const model::Game& game = get_game_by_file_path(games, path);
        QCOMPARE(game.assets().videoList(), QStringList({
            QStringLiteral("file::/custom_assets_multi/videos/a.mp4"),
            QStringLiteral("file::/custom_assets_multi/videos/b.mp4"),
            QStringLiteral("file::/custom_assets_multi/videos/c.mp4"),
        }));
    }
}

void test_PegasusProvider::custom_directories()
{
    QTest::ignoreMessage(QtInfoMsg, PATHMSG("Metafiles: Found `%1`", ":/custom_dirs/coll/metadata.txt"));

    providers::SearchContext sctx({QStringLiteral(":/custom_dirs/coll")});
    providers::pegasus::PegasusProvider().run(sctx);
    const auto [collections, games] = sctx.finalize(this);

    QCOMPARE(collections.size(), 2);
    QCOMPARE(games.size(), 5);
    QVERIFY(has_collection(collections, QStringLiteral("x-files")));
    QVERIFY(has_collection(collections, QStringLiteral("y-files")));
    QCOMPARE(get_collection(collections, QStringLiteral("x-files")).gamesConst().size(), 4);
    QCOMPARE(get_collection(collections, QStringLiteral("y-files")).gamesConst().size(), 1);

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
    verify_collected_files(collections, coll_files_map);
}

void test_PegasusProvider::multifile()
{
    QTest::ignoreMessage(QtInfoMsg, PATHMSG("Metafiles: Found `%1`", ":/multifile/metadata.txt"));

    providers::SearchContext sctx({QStringLiteral(":/multifile")});
    providers::pegasus::PegasusProvider().run(sctx);
    const auto [collections, games] = sctx.finalize(this);

    QCOMPARE(collections.size(),  1);
    QCOMPARE(games.size(), 2);
    QCOMPARE(games.at(0)->filesConst().size(), 2);
    QCOMPARE(games.at(1)->filesConst().size(), 1);

    const QVector<model::Game*>& child_vec = collections.first()->gamesConst();
    QVERIFY(std::find(child_vec.cbegin(), child_vec.cend(), games.at(0)) != child_vec.cend());
    QVERIFY(std::find(child_vec.cbegin(), child_vec.cend(), games.at(1)) != child_vec.cend());
}

void test_PegasusProvider::multicoll()
{
    QTest::ignoreMessage(QtInfoMsg, PATHMSG("Metafiles: Found `%1`", ":/multicoll/c1/metadata.txt"));
    QTest::ignoreMessage(QtInfoMsg, PATHMSG("Metafiles: Found `%1`", ":/multicoll/c2/metadata.txt"));
    QTest::ignoreMessage(QtInfoMsg, PATHMSG("Metafiles: Found `%1`", ":/multicoll/games/metadata.txt"));

    providers::SearchContext sctx({
        QStringLiteral(":/multicoll/c1"),
        QStringLiteral(":/multicoll/c2"),
        QStringLiteral(":/multicoll/games"),
    });
    providers::pegasus::PegasusProvider().run(sctx);
    const auto [collections, games] = sctx.finalize(this);

    QCOMPARE(collections.size(),  2);
    QCOMPARE(games.size(), 1);
    QCOMPARE(collections.first()->gamesConst().first(), collections.last()->gamesConst().first());

    const model::Game& game = *games.first();
    QCOMPARE(game.title(), QStringLiteral("My Game"));
    QCOMPARE(game.summary(), QStringLiteral("Some Summary"));
    QCOMPARE(game.description(), QStringLiteral("Some Description"));
    QCOMPARE(game.developerStr(), QStringLiteral("My Developer"));
    QCOMPARE(game.publisherStr(), QStringLiteral("My Publisher"));
    QCOMPARE(game.genreStr(), QStringLiteral("My Genre"));
    QCOMPARE(game.assets().boxFront(), QStringLiteral("file::/multicoll/games/game.jpg"));
}

void test_PegasusProvider::nonASCII()
{
    struct TestEntry {
        QString title;
        QString filename;
        QString desc;
    };
    // It's 2019 and yet some compilers still have Unicode troubles...
    const QVector<TestEntry> entries {
        {
            "AsciiGame",
            "ascii.ext",
            "A simple ASCII filename",
        }, {
            u8"\u00C1rv\u00EDzt\u0171r\u0151", // Árvíztűrő
            u8"\u00E1rv\u00EDzt\u0171r\u0151.ext", // árvíztűrő.ext
            u8"\u00C1rv\u00EDzt\u0171r\u0151 t\u00FCk\u00F6rf\u00FAr\u00F3g\u00E9p", // Árvíztűrő tükörfúrógép
        }, {
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


    const QString ignored_msg = QStringLiteral("Metafiles: Found `%1`")
        .arg(QDir::toNativeSeparators(tempdir.path() + QStringLiteral("/metadata.txt")));
    QTest::ignoreMessage(QtInfoMsg, qUtf8Printable(ignored_msg));

    providers::SearchContext sctx({tempdir.path()});
    providers::pegasus::PegasusProvider().run(sctx);
    const auto [collections, games] = sctx.finalize(this);

    QCOMPARE(collections.size(), 1);
    QCOMPARE(games.size(), entries.size());

    for (const TestEntry& expected : entries) {
        const auto it = std::find_if(
            games.cbegin(),
            games.cend(),
            [&expected](const model::Game* const game){ return game->title() == expected.title; });
        QVERIFY(it != games.cend());

        const model::Game& game = *(*it);
        QCOMPARE(game.title(), expected.title);
        QCOMPARE(game.filesConst().size(), 1);
        QCOMPARE(game.filesConst().front()->fileinfo().absoluteFilePath(), tempdir.path() + "/" + expected.filename);
        QCOMPARE(game.filesConst().front()->fileinfo().exists(), true);
        QCOMPARE(game.description(), expected.desc);

        // FIXME: This fails on macOS,
        // because the files are created under /var/.../<appname>/
        // but the media searching happens in /private/var/.../<appname>/
        // for some reason
#ifndef Q_OS_DARWIN
        // TODO: Move this to the Media provider test
        // const QString expected_asset_path = QUrl::fromLocalFile(tempdir.path() + "/media/" + expected.title + "/box_front.png").toString();
        // QCOMPARE(game.assets().boxFront(), expected_asset_path);
#endif
    }
}

void test_PegasusProvider::relative_files_only()
{
    QTest::ignoreMessage(QtInfoMsg, PATHMSG("Metafiles: Found `%1`", ":/relative_files/coll/metadata.txt"));

    providers::SearchContext sctx({QStringLiteral(":/relative_files/coll")});
    providers::pegasus::PegasusProvider().run(sctx);
    const auto [collections, games] = sctx.finalize(this);

    QCOMPARE(collections.size(), 1);
    QVERIFY(has_collection(collections, QStringLiteral("myfiles")));

    const HashMap<QString, QStringList> coll_files_map {
        { QStringLiteral("myfiles"), {
            { ":/relative_files/coll/../a/game_a.ext" },
            { ":/relative_files/coll/game_here.ext" },
            { ":/relative_files/coll/b/game_b.ext" },
            { ":/relative_files/coll/./c/game_c.ext" },
        }},
    };
    verify_collected_files(collections, coll_files_map);
}

void test_PegasusProvider::relative_files_with_dirs()
{
    QTest::ignoreMessage(QtInfoMsg, PATHMSG("Metafiles: Found `%1`", ":/relative_files_with_dirs/coll/metadata.txt"));

    providers::SearchContext sctx({QStringLiteral(":/relative_files_with_dirs/coll")});
    providers::pegasus::PegasusProvider().run(sctx);
    const auto [collections, games] = sctx.finalize(this);

    QCOMPARE(collections.size(), 1);
    QVERIFY(has_collection(collections, QStringLiteral("myfiles")));

    const HashMap<QString, QStringList> coll_files_map {
        { QStringLiteral("myfiles"), {
            { ":/relative_files_with_dirs/coll/../a/game_a.x" },
            { ":/relative_files_with_dirs/coll/../b/game_b.x" },
            { ":/relative_files_with_dirs/coll/game_here.x" },
        }},
    };
    verify_collected_files(collections, coll_files_map);
}


QTEST_MAIN(test_PegasusProvider)
#include "test_PegasusProvider.moc"
