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
#include "providers/playnite/PlayniteProvider.h"


namespace {
const model::Game* get_game_ptr_by_file_path(const QVector<model::Game*>& list, const QString& path)
{
    const auto it = std::find_if(
        list.cbegin(),
        list.cend(),
        [&path](const model::Game* const game) { return std::any_of(
            game->filesConst().cbegin(),
            game->filesConst().cend(),
            [&path](const model::GameFile* gf) { return gf->fileinfo().canonicalFilePath() == path; });
        });
    return it != list.cend()
        ? *it
        : nullptr;
}
} // namespace


class test_PlayniteProvider : public QObject {
    Q_OBJECT

private slots:
    void initTestCase() {
        Log::init_qttest();
    }

    void empty();
    void basic();
};

void test_PlayniteProvider::empty()
{
    QTest::ignoreMessage(QtWarningMsg, "Playnite: No games found");
    QTest::ignoreMessage(QtInfoMsg, "Playnite: Looking for installation at `:\\empty\\`");

    providers::SearchContext sctx;
    providers::playnite::PlayniteProvider provider;
    provider
        .setOption(QStringLiteral("installdir"), QStringLiteral(":/empty"))
        .run(sctx);
    const auto [collections, games] = sctx.finalize(this);

    QVERIFY(games.isEmpty());
    QVERIFY(collections.isEmpty());
}

void test_PlayniteProvider::basic()
{
    QTest::ignoreMessage(QtInfoMsg, R"(Playnite: Looking for installation at `:\basic\Playnite\`)");

    providers::SearchContext sctx;
    providers::playnite::PlayniteProvider provider;
    provider
        .setOption(QStringLiteral("installdir"), QStringLiteral(":/basic/Playnite"))
        .run(sctx);

    const auto [collections, games] = sctx.finalize(this);

    QCOMPARE(collections.size(), 2);
    const auto coll_it = std::find_if(
        collections.cbegin(),
        collections.cend(),
        [](const model::Collection* const collection) { return collection->name() == QLatin1String("Nintendo Entertainment System"); });
    QVERIFY(coll_it != collections.cend());

    QCOMPARE(games.size(), 2);

    const auto entry1_filepath = QStringLiteral(":/basic/game/Test Bros (JU) [!].zip");
    const model::Game* const entry1_game_ptr = get_game_ptr_by_file_path(games, entry1_filepath);
    QVERIFY(entry1_game_ptr != nullptr);
    const model::Game& game = *entry1_game_ptr;

    QCOMPARE(game.title(), QStringLiteral("Super Mario Bros."));
    QCOMPARE(game.sortBy(), QStringLiteral("Test Bros (JU) [!]"));
    QCOMPARE(game.summary(), QStringLiteral("Some description here!"));
    QCOMPARE(game.description(), QStringLiteral("Some description here!"));
    QCOMPARE(game.releaseDate(), QDate(1985, 9, 13));
    QCOMPARE(game.rating(), 0.7f);
    QCOMPARE(game.developerListConst(), { QStringLiteral("Nintendo") });
    QCOMPARE(game.publisherListConst(), { QStringLiteral("Nintendo, Inc.") });
    QStringList genres = game.genreListConst();
    genres.sort();
    QCOMPARE(genres, QStringList({ QStringLiteral("Multiplayer"), QStringLiteral("Platform") }));

    QCOMPARE(game.launchCmd(), QStringLiteral(":/basic/emu/nestopia.exe --exec={file.path} --batch"));
    QCOMPARE(game.launchWorkdir(), QStringLiteral(":/basic/emu"));

    const model::Collection& coll = **coll_it;
    QCOMPARE(coll.gamesConst().size(), 1);
    QCOMPARE(coll.gamesConst().first(), &game);
    QCOMPARE(game.collectionsConst().size(), 1);
    QCOMPARE(game.collectionsConst().first(), &coll);
    QCOMPARE(game.filesConst().size(), 1);


    QCOMPARE(game.filesConst().first()->name(), QStringLiteral("Test Bros (JU) [!]"));
    QCOMPARE(game.filesConst().first()->path(), entry1_filepath);
}


QTEST_MAIN(test_PlayniteProvider)
#include "test_PlayniteProvider.moc"
