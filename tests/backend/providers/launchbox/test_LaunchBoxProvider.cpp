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
#include "model/gaming/Collection.h"
#include "model/gaming/Game.h"
#include "model/gaming/GameFile.h"
#include "providers/SearchContext.h"
#include "providers/launchbox/LaunchBoxProvider.h"


namespace {
const model::Game* get_game_ptr_by_file_path(const std::vector<model::Game*>& list, const QString& path)
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
} // namespace


class test_LaunchBoxProvider : public QObject {
    Q_OBJECT

private slots:
    void initTestCase() {
        Log::init_qttest();
    }

    void empty();
    void basic();
};

void test_LaunchBoxProvider::empty()
{
    QTest::ignoreMessage(QtInfoMsg, "LaunchBox: Looking for installation at `:\\empty`");
    QTest::ignoreMessage(QtWarningMsg, "LaunchBox: Could not open `:\\empty\\Data\\Platforms.xml`");
    QTest::ignoreMessage(QtWarningMsg, "LaunchBox: No platforms found");

    providers::SearchContext sctx;
    providers::launchbox::LaunchboxProvider provider;
    provider
        .setOption(QStringLiteral("installdir"), QStringLiteral(":/empty"))
        .run(sctx);
    const auto [collections, games] = sctx.finalize(this);

    QVERIFY(games.empty());
    QVERIFY(collections.isEmpty());
}

void test_LaunchBoxProvider::basic()
{
    QTest::ignoreMessage(QtInfoMsg, "LaunchBox: Looking for installation at `:\\basic\\LaunchBox`");

    providers::SearchContext sctx;
    providers::launchbox::LaunchboxProvider provider;
    provider
        .setOption(QStringLiteral("installdir"), QStringLiteral(":/basic/LaunchBox"))
        .run(sctx);
    const auto [collections, games] = sctx.finalize(this);


    QCOMPARE(collections.size(), 1);
    const auto coll_it = std::find_if(
        collections.cbegin(),
        collections.cend(),
        [](const model::Collection* const collection){ return collection->name() == QStringLiteral("Nintendo Entertainment System"); });
    QVERIFY(coll_it != collections.cend());

    const model::Collection& coll = **coll_it;
    QCOMPARE(coll.sortBy(), QLatin1String("Nintendo 1985"));


    QCOMPARE(games.size(), 1);
    const auto entry1_filepath = QStringLiteral(":/basic/game/Test Bros (JU) [!].zip");
    const auto entry2_filepath = QStringLiteral(":/basic/game/Test Bros Something.zip");
    const model::Game* const entry1_game_ptr = get_game_ptr_by_file_path(games, entry1_filepath);
    const model::Game* const entry2_game_ptr = get_game_ptr_by_file_path(games, entry2_filepath);
    QVERIFY(entry1_game_ptr != nullptr);
    QVERIFY(entry2_game_ptr != nullptr);
    QVERIFY(entry2_game_ptr == entry1_game_ptr);

    const model::Game& game = *entry1_game_ptr;


    QCOMPARE(game.title(), QStringLiteral("Super Mario Bros."));
    QCOMPARE(game.sortBy(), QStringLiteral("Super Mario 1"));
    QCOMPARE(game.summary(), QStringLiteral("Some description here!"));
    QCOMPARE(game.description(), QStringLiteral("Some description here!"));
    QCOMPARE(game.releaseDate(), QDate(1985, 9, 13));
    // QCOMPARE(game.playerCount(), 2);
    QCOMPARE(game.rating(), 4.572131f / 5.f);

    QCOMPARE(game.developerListConst(), { QStringLiteral("Nintendo") });
    QCOMPARE(game.publisherListConst(), { QStringLiteral("Nintendo") });
    QStringList genres = game.genreListConst();
    genres.sort();
    QCOMPARE(genres, QStringList({ QStringLiteral("Multiplayer"), QStringLiteral("Platform") }));
    // QCOMPARE(game.tagListConst(), { QStringLiteral("") });

    // QCOMPARE(game.playCount(), 15);
    // QCOMPARE(game.isFavorite(), true);

    QCOMPARE(game.launchCmd(), QStringLiteral("\":/basic/emu/nestopia.exe\" -some -flag {file.path}"));
    QCOMPARE(game.launchWorkdir(), QStringLiteral(":/basic/emu"));


    QCOMPARE(coll.gameList()->entries().size(), 1);
    QCOMPARE(coll.gameList()->entries().front(), &game);
    QCOMPARE(game.collectionsConst().size(), 1);
    QCOMPARE(game.collectionsConst().first(), &coll);
    QCOMPARE(game.filesConst().size(), 2);


    QCOMPARE(game.filesConst().first()->name(), QStringLiteral("Entry 1..."));
    QCOMPARE(game.filesConst().first()->path(), entry1_filepath);
    // QCOMPARE(game.filesConst().first()->playCount(), 5);
    QCOMPARE(game.filesConst().last()->name(), QStringLiteral("Entry 2..."));
    QCOMPARE(game.filesConst().last()->path(), entry2_filepath);
    // QCOMPARE(game.filesConst().last()->playCount(), 10);
}


QTEST_MAIN(test_LaunchBoxProvider)
#include "test_LaunchBoxProvider.moc"
