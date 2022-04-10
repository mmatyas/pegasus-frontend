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
#include "providers/pegasus_media/MediaProvider.h"
#include "utils/StdHelpers.h"

#include <QString>
#include <QStringList>


namespace {
bool has_collection(const std::vector<model::Collection*>& list, const QString& name)
{
    const auto it = std::find_if(list.cbegin(), list.cend(),
        [&name](const model::Collection* const ptr){ return ptr->name() == name; });
    return it != list.cend();
}

const model::Game* get_game_ptr_by_file_path(const std::vector<model::Game*>& list, const QString& path)
{
    const auto it = std::find_if(
        list.cbegin(),
        list.cend(),
        [&path](const model::Game* const game){ return std::any_of(
            game->filesModel()->entries().cbegin(),
            game->filesModel()->entries().cend(),
            [&path](const model::GameFile* const gf){ return gf->fileinfo().canonicalFilePath() == path; });
        });
    return it != list.cend()
        ? *it
        : nullptr;
}

const model::Game& get_game_by_file_path(const std::vector<model::Game*>& list, const QString& path)
{
    const model::Game* ptr = get_game_ptr_by_file_path(list, path);
    Q_ASSERT(ptr != nullptr);
    return *ptr;
}

bool has_game_file(const std::vector<model::Game*>& list, const QString& path)
{
    return get_game_ptr_by_file_path(list, path) != nullptr;
}
} // namespace


class test_PegasusMediaProvider : public QObject {
    Q_OBJECT

private slots:
    void initTestCase() {
        Log::init_qttest();
    }

    void asset_search();
    void asset_search_by_title();
    void asset_search_multifile();
    void separate_media_dirs();
};

void test_PegasusMediaProvider::asset_search()
{
    const QString display_path = QDir::toNativeSeparators(QStringLiteral(":/asset_search/metadata.txt"));
    QTest::ignoreMessage(QtInfoMsg, qUtf8Printable(QStringLiteral("Metafiles: Found `%1`").arg(display_path)));

    providers::SearchContext sctx({QStringLiteral(":/asset_search")});
    providers::pegasus::PegasusProvider().run(sctx);
    providers::media::MediaProvider().run(sctx);
    const auto [collections, games] = sctx.finalize(this);

    const QString collection_name(QStringLiteral("mygames"));
    QCOMPARE(collections.size(), 1);
    QVERIFY(has_collection(collections, collection_name));
    QCOMPARE(games.size(), 4);

    {
        const auto path = QStringLiteral(":/asset_search/mygame1.ext");
        QVERIFY(has_game_file(games, path));
        const model::Game& game = get_game_by_file_path(games, path);

        QCOMPARE(game.assets().boxFront(), QStringLiteral("file::/asset_search/media/mygame1/box_front.png"));
        QCOMPARE(game.assets().videoList(), { QStringLiteral("file::/asset_search/media/mygame1/video.mp4") });
    }
    {
        const auto path = QStringLiteral(":/asset_search/mygame3.ext");
        QVERIFY(has_game_file(games, path));
        const model::Game& game = get_game_by_file_path(games, path);

        QCOMPARE(game.assets().screenshotList(),
            { QStringLiteral("file::/asset_search/media/mygame3/screenshot.jpg") });
        QCOMPARE(game.assets().music(),
            QStringLiteral("file::/asset_search/media/mygame3/music.mp3"));
    }
    {
        const auto path = QStringLiteral(":/asset_search/subdir/mygame4.ext");
        QVERIFY(has_game_file(games, path));
        const model::Game& game = get_game_by_file_path(games, path);

        QCOMPARE(game.assets().background(),
            QStringLiteral("file::/asset_search/media/subdir/mygame4/background.png"));
    }
}

void test_PegasusMediaProvider::asset_search_by_title()
{
    const QString display_path = QDir::toNativeSeparators(QStringLiteral(":/asset_search_by_title/metadata.txt"));
    QTest::ignoreMessage(QtInfoMsg, qUtf8Printable(QStringLiteral("Metafiles: Found `%1`").arg(display_path)));

    providers::SearchContext sctx({QStringLiteral(":/asset_search_by_title")});
    providers::pegasus::PegasusProvider().run(sctx);
    providers::media::MediaProvider().run(sctx);
    const auto [collections, games] = sctx.finalize(this);

    const QString collection_name(QStringLiteral("mygames"));
    QCOMPARE(collections.size(), 1);
    QVERIFY(has_collection(collections, collection_name));
    QCOMPARE(games.size(), 1);

    const QString path = QStringLiteral(":/asset_search_by_title/mygame.ext");
    QVERIFY(has_game_file(games, path));
    const model::Game& game = get_game_by_file_path(games, path);

    QCOMPARE(game.assets().boxFront(),
        QStringLiteral("file::/asset_search_by_title/media/My Precious Game/box_front.png"));
    QCOMPARE(game.assets().videoList(),
        { QStringLiteral("file::/asset_search_by_title/media/My Precious Game/video.mp4") });
}

void test_PegasusMediaProvider::asset_search_multifile()
{
    const QString display_path = QDir::toNativeSeparators(QStringLiteral(":/asset_search_multifile/metadata.txt"));
    QTest::ignoreMessage(QtInfoMsg, qUtf8Printable(QStringLiteral("Metafiles: Found `%1`").arg(display_path)));

    providers::SearchContext sctx({QStringLiteral(":/asset_search_multifile")});
    providers::pegasus::PegasusProvider().run(sctx);
    providers::media::MediaProvider().run(sctx);
    const auto [collections, games] = sctx.finalize(this);

    const QString collection_name(QStringLiteral("mygames"));
    QCOMPARE(collections.size(), 1);
    QVERIFY(has_collection(collections, collection_name));
    QCOMPARE(games.size(), 1);

    auto path = QStringLiteral(":/asset_search_multifile/mygame.ext");
    QVERIFY(has_game_file(games, path));
    const model::Game& game = get_game_by_file_path(games, path);

    const QStringList actual = [&game]{
        QStringList list = game.assets().screenshotList();
        list.sort();
        return list;
    }();
    const QStringList expected {
        QStringLiteral("file::/asset_search_multifile/media/mygame/screenshot01.png"),
        QStringLiteral("file::/asset_search_multifile/media/mygame/screenshot02.png"),
    };
    QCOMPARE(actual, expected);
}

void test_PegasusMediaProvider::separate_media_dirs()
{
    // NOTE: see issue 407

    const QString display_path = QDir::toNativeSeparators(QStringLiteral(":/separate_media_dirs/metadata/metadata.pegasus.txt"));
    QTest::ignoreMessage(QtInfoMsg, qUtf8Printable(QStringLiteral("Metafiles: Found `%1`").arg(display_path)));

    providers::SearchContext sctx({QStringLiteral(":/separate_media_dirs/metadata")});
    providers::pegasus::PegasusProvider().run(sctx);
    providers::media::MediaProvider().run(sctx);
    const auto [collections, games] = sctx.finalize(this);

    QCOMPARE(collections.size(), 2);
    QCOMPARE(games.size(), 2);

    QCOMPARE(games.at(0)->assets().boxFront(), QStringLiteral("file::/separate_media_dirs/games-a/media/Game 1/box_front.png"));
    QCOMPARE(games.at(1)->assets().boxFront(), QStringLiteral("file::/separate_media_dirs/games-b/media/Game 2/box_front.png"));
}


QTEST_MAIN(test_PegasusMediaProvider)
#include "test_PegasusMediaProvider.moc"
