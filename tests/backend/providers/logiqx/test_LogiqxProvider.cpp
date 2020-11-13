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
#include "providers/logiqx/LogiqxProvider.h"

#include <QString>
#include <QStringList>


#define PATHMSG(msg, path) qUtf8Printable( \
    QStringLiteral(msg) \
        .arg(QDir::toNativeSeparators(QStringLiteral(path))))

#define PATH2MSG(msg, path1, path2) qUtf8Printable( \
    QStringLiteral(msg) \
        .arg(QDir::toNativeSeparators(QStringLiteral(path1))) \
        .arg(QDir::toNativeSeparators(QStringLiteral(path2))))


class test_LogiqxProvider : public QObject {
    Q_OBJECT

private slots:
    void initTestCase() {
        Log::init_qttest();
    }

    void faulty();
    void malformed();
    void simple();
};


void test_LogiqxProvider::faulty()
{
    QTest::ignoreMessage(QtWarningMsg, PATHMSG("Logiqx: `%1` doesn't seem to be a valid XML file, ignored", ":/faulty/empty.dat"));
    QTest::ignoreMessage(QtWarningMsg, PATHMSG("Logiqx: `%1` seems to be a valid XML file, but doesn't have a DOCTYPE declaration, ignored", ":/faulty/no_doctype.dat"));
    QTest::ignoreMessage(QtWarningMsg, PATHMSG("Logiqx: `%1` is not declared as a Logiqx XML file, ignored", ":/faulty/not_logiqx.dat"));
    QTest::ignoreMessage(QtWarningMsg, PATHMSG("Logiqx: `%1` seems to be a Logiqx file, but doesn't start with a `datafile` root element", ":/faulty/incorrect_root.dat"));

    const QStringList game_dirs { QStringLiteral(":/faulty") };

    providers::SearchContext sctx(game_dirs);
    providers::logiqx::LogiqxProvider().run(sctx);
    auto [collections, games] = sctx.finalize(this);

    QCOMPARE(collections.size(), 0);
    QCOMPARE(games.size(), 0);
}


void test_LogiqxProvider::malformed()
{
    QTest::ignoreMessage(QtInfoMsg, PATHMSG("Logiqx: Found `%1`", ":/malformed/bad_header.dat"));
    QTest::ignoreMessage(QtWarningMsg, PATHMSG("Logiqx: `%1` has no `name` field in its `header` entry", ":/malformed/bad_header.dat"));

    QTest::ignoreMessage(QtInfoMsg, PATHMSG("Logiqx: Found `%1`", ":/malformed/bad_games.dat"));
    QTest::ignoreMessage(QtWarningMsg, PATHMSG("Logiqx: The `game` element in `%1` at line 8 has an empty or missing `name` attribute, entry ignored", ":/malformed/bad_games.dat"));
    QTest::ignoreMessage(QtWarningMsg, PATHMSG("Logiqx: The `game` element in `%1` at line 11 has no valid `rom` fields, game ignored", ":/malformed/bad_games.dat"));
    QTest::ignoreMessage(QtWarningMsg, PATHMSG("Logiqx: The `year` element in `%1` at line 15 has an invalid value, ignored", ":/malformed/bad_games.dat"));
    QTest::ignoreMessage(QtWarningMsg, PATHMSG("Logiqx: The `rom` element in `%1` at line 19 has an empty or missing `name` attribute, ignored", ":/malformed/bad_games.dat"));
    QTest::ignoreMessage(QtWarningMsg, PATHMSG("Logiqx: The `game` element in `%1` at line 18 has no valid `rom` fields, game ignored", ":/malformed/bad_games.dat"));
    QTest::ignoreMessage(QtWarningMsg, PATHMSG("Logiqx: The `rom` element in `%1` at line 23 seems to be a duplicate entry, ignored", ":/malformed/bad_games.dat"));
    QTest::ignoreMessage(QtWarningMsg, PATHMSG("Logiqx: The `game` element in `%1` at line 25 has no valid `rom` fields, game ignored", ":/malformed/bad_games.dat"));
    QTest::ignoreMessage(QtWarningMsg, PATH2MSG("Logiqx: The `rom` element in `%1` at line 28 refers to file `%2`, which doesn't seem to exist", ":/malformed/bad_games.dat", ":/malformed/nothing.ext"));
    QTest::ignoreMessage(QtWarningMsg, PATHMSG("Logiqx: The `game` element in `%1` at line 27 has no valid `rom` fields, game ignored", ":/malformed/bad_games.dat"));

    const QStringList game_dirs { QStringLiteral(":/malformed") };

    providers::SearchContext sctx(game_dirs);
    providers::logiqx::LogiqxProvider().run(sctx);
    auto [collections, games] = sctx.finalize(this);

    QCOMPARE(collections.size(), 1);
    QCOMPARE(collections.front()->name(), QStringLiteral("My Platform"));

    QCOMPARE(games.size(), 2);
    const auto game_badyear_it = std::find_if(games.cbegin(), games.cend(),
        [](const model::Game* const game){ return game->title() == QStringLiteral("Bad year"); });
    const auto game_samefile_it = std::find_if(games.cbegin(), games.cend(),
        [](const model::Game* const game){ return game->title() == QStringLiteral("Same file"); });
    QVERIFY(game_badyear_it != games.cend());
    QVERIFY(game_samefile_it != games.cend());
}


void test_LogiqxProvider::simple()
{
    QTest::ignoreMessage(QtInfoMsg, PATHMSG("Logiqx: Found `%1`", ":/simple/something.dat"));

    const QStringList game_dirs { QStringLiteral(":/simple") };

    providers::SearchContext sctx(game_dirs);
    providers::logiqx::LogiqxProvider().run(sctx);
    auto [collections, games] = sctx.finalize(this);

    QCOMPARE(collections.size(), 1);
    QCOMPARE(collections.front()->name(), QStringLiteral("My Platform"));

    QCOMPARE(games.size(), 2);
    const auto game1_it = std::find_if(games.cbegin(), games.cend(),
        [](const model::Game* const game){ return game->title() == QStringLiteral("Game 1"); });
    const auto game2_it = std::find_if(games.cbegin(), games.cend(),
        [](const model::Game* const game){ return game->title() == QStringLiteral("Game 2"); });
    QVERIFY(game1_it != games.cend());
    QVERIFY(game2_it != games.cend());

    const model::Game& game1 = **game1_it;
    QCOMPARE(game1.developerStr(), QStringLiteral("Game Corp."));
    QCOMPARE(game1.releaseYear(), 1990);
    QCOMPARE(game1.description(), QStringLiteral("The description of Game 1."));
    QCOMPARE(game1.filesConst().size(), 1);
    QCOMPARE(game1.filesConst().front()->path(), QStringLiteral(":/simple/Game 1.ext"));

    const model::Game& game2 = **game2_it;
    QCOMPARE(game2.developerStr(), QStringLiteral("Game Corp."));
    QCOMPARE(game2.releaseYear(), 2000);
    QCOMPARE(game2.description(), QStringLiteral("The description of Game 2."));
    QCOMPARE(game2.filesConst().size(), 2);
    QCOMPARE(game2.filesConst().front()->path(), QStringLiteral(":/simple/Game 2x1.ext"));
    QCOMPARE(game2.filesConst().back()->path(), QStringLiteral(":/simple/Game 2x2.ext"));
}


QTEST_MAIN(test_LogiqxProvider)
#include "test_LogiqxProvider.moc"
