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

#include "model/gaming/Collection.h"
#include "model/gaming/Game.h"
#include "providers/launchbox/LaunchBoxProvider.h"


namespace {

void finalize_lists(providers::SearchContext& sctx)
{
    for (auto& entry : sctx.games)
        entry.second->finalize();
    for (auto& entry : sctx.collections)
        entry.second->finalize();
}

} // namespace


class test_LaunchBoxProvider : public QObject {
    Q_OBJECT

private slots:
    void empty();
    void basic();
};

void test_LaunchBoxProvider::empty()
{
    QTest::ignoreMessage(QtWarningMsg, "LaunchBox: could not open `Data\\Platforms.xml`");
    QTest::ignoreMessage(QtWarningMsg, "LaunchBox: no platforms found");

    providers::SearchContext sctx;
    providers::launchbox::LaunchboxProvider provider;
    provider
        .setOption(QStringLiteral("installdir"), QStringLiteral(":/empty"))
        .load()
        .findLists(sctx);

    QVERIFY(sctx.games.empty());
    QVERIFY(sctx.collections.empty());
}

void test_LaunchBoxProvider::basic()
{
    providers::SearchContext sctx;
    providers::launchbox::LaunchboxProvider provider;
    provider
        .setOption(QStringLiteral("installdir"), QStringLiteral(":/basic/LaunchBox"))
        .load()
        .findLists(sctx);
    finalize_lists(sctx);


    QCOMPARE(static_cast<int>(sctx.collections.size()), 1);
    const auto coll_it = sctx.collections.find(QStringLiteral("Nintendo Entertainment System"));
    QVERIFY(coll_it != sctx.collections.cend());

    const model::Collection& coll = *coll_it->second;
    QCOMPARE(coll.name(), QStringLiteral("Nintendo Entertainment System"));


    QCOMPARE(static_cast<int>(sctx.path_to_gameid.size()), 2);
    const auto file1_it = sctx.path_to_gameid.find(QStringLiteral(":/basic/LaunchBox/../game/Test Bros (JU) [!].zip"));
    const auto file2_it = sctx.path_to_gameid.find(QStringLiteral(":/basic/LaunchBox/../game/Test Bros Something.zip"));
    QVERIFY(file1_it != sctx.path_to_gameid.cend());
    QVERIFY(file1_it->second == file2_it->second);
    const size_t game_id = file1_it->second;

    QCOMPARE(static_cast<int>(sctx.games.size()), 1);
    const auto game_it = sctx.games.find(game_id);
    QVERIFY(game_it != sctx.games.cend());

    const model::Game& game = *game_it->second;
    QCOMPARE(game.title(), QStringLiteral("Super Mario Bros."));
    QCOMPARE(game.sortBy(), QStringLiteral("Super Mario Bros."));
    QCOMPARE(game.summary(), QStringLiteral("Some description here!"));
    QCOMPARE(game.description(), QStringLiteral("Some description here!"));
    QCOMPARE(game.releaseDate(), QDate(1985, 9, 13));
    // QCOMPARE(game.playerCount(), 2);
    QCOMPARE(game.rating(), 4.572131f);

    QCOMPARE(game.developerListConst(), { QStringLiteral("Nintendo") });
    QCOMPARE(game.publisherListConst(), { QStringLiteral("Nintendo") });
    QStringList genres = game.genreListConst();
    genres.sort();
    QCOMPARE(genres, QStringList({ QStringLiteral("Multiplayer"), QStringLiteral("Platform") }));
    // QCOMPARE(game.tagListConst(), { QStringLiteral("") });

    // QCOMPARE(game.playCount(), 15);
    // QCOMPARE(game.isFavorite(), true);

    QCOMPARE(game.launchCmd(), QStringLiteral("\":/basic/LaunchBox/../emu/nestopia.exe\" -some -flag {file.path}"));
    QCOMPARE(game.launchWorkdir(), QStringLiteral(":/basic/LaunchBox/../emu"));


    QCOMPARE(coll.gamesConst().size(), 1);
    QCOMPARE(coll.gamesConst().first(), game_it->second);
    QCOMPARE(game.collectionsConst().size(), 1);
    QCOMPARE(game.collectionsConst().first(), coll_it->second);
    QCOMPARE(game.filesConst().size(), 2);


    QCOMPARE(game.filesConst().first()->name(), QStringLiteral("Entry 1..."));
    QCOMPARE(game.filesConst().first()->path(), file1_it->first);
    // QCOMPARE(game.filesConst().first()->playCount(), 5);
    QCOMPARE(game.filesConst().last()->name(), QStringLiteral("Entry 2..."));
    QCOMPARE(game.filesConst().last()->path(), file2_it->first);
    // QCOMPARE(game.filesConst().last()->playCount(), 10);
}


QTEST_MAIN(test_LaunchBoxProvider)
#include "test_LaunchBoxProvider.moc"
