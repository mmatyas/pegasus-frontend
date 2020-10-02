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
#include "providers/SearchContext.h"
#include "providers/logiqx/LogiqxProvider.h"
#include "utils/HashMap.h"
#include "utils/StdHelpers.h"

#include <QString>
#include <QStringList>


class test_LogiqxProvider : public QObject {
    Q_OBJECT

private slots:
    void faulty();
    void malformed();
    void simple();
};


void test_LogiqxProvider::faulty()
{
    using Rx = QRegularExpression;
    QTest::ignoreMessage(QtInfoMsg, Rx("Logiqx: `:.faulty.empty\\.dat` doesn't seem to be a valid XML file, ignored"));
    QTest::ignoreMessage(QtInfoMsg, Rx("Logiqx: `:.faulty.no_doctype\\.dat` seems to be a valid XML file, but doesn't have a DOCTYPE declaration, ignored"));
    QTest::ignoreMessage(QtInfoMsg, Rx("Logiqx: `:.faulty.not_logiqx\\.dat` is not declared as a Logiqx XML file, ignored"));
    QTest::ignoreMessage(QtWarningMsg, Rx("Logiqx: `:.faulty.incorrect_root\\.dat` seems to be a Logiqx file, but doesn't start with a `datafile` root element"));

    const QStringList game_dirs { QStringLiteral(":/faulty") };

    providers::SearchContext sctx;
    providers::logiqx::LogiqxProvider provider;
    provider
        .load_with_config(game_dirs)
        .findLists(sctx);

    // TODO: C++17
    QVector<model::Collection*> collections;
    QVector<model::Game*> games;
    std::tie(collections, games) = sctx
        .finalize_lists()
        .consume();

    QCOMPARE(collections.size(), 0);
    QCOMPARE(games.size(), 0);
}


void test_LogiqxProvider::malformed()
{
    using Rx = QRegularExpression;
    QTest::ignoreMessage(QtInfoMsg, Rx("Logiqx: Found `:.malformed.bad_games\\.dat`"));
    QTest::ignoreMessage(QtInfoMsg, Rx("Logiqx: Found `:.malformed.bad_header\\.dat`"));
    QTest::ignoreMessage(QtWarningMsg, Rx("Logiqx: `:.malformed.bad_header\\.dat` has no `name` field in its `header` entry"));
    QTest::ignoreMessage(QtWarningMsg, Rx("Logiqx: The `game` element in `:.malformed.bad_games\\.dat` at line 8 has an empty or missing `name` attribute, entry ignored"));
    QTest::ignoreMessage(QtWarningMsg, Rx("Logiqx: The `year` element in `:.malformed.bad_games\\.dat` at line 15 has an invalid value, ignored"));
    QTest::ignoreMessage(QtWarningMsg, Rx("Logiqx: The `rom` element in `:.malformed.bad_games\\.dat` at line 19 has an empty or missing `name` attribute, ignored"));
    QTest::ignoreMessage(QtWarningMsg, Rx("Logiqx: The `rom` element in `:.malformed.bad_games\\.dat` at line 23 seems to be a duplicate entry, ignored"));
    QTest::ignoreMessage(QtWarningMsg, Rx("Logiqx: The `rom` element in `:.malformed.bad_games\\.dat` at line 28 refers to file `:.malformed.nothing\\.ext`, which doesn't seem to exist"));
    QTest::ignoreMessage(QtWarningMsg, "No files defined for game 'No valid fields', ignored");
    QTest::ignoreMessage(QtWarningMsg, "No files defined for game 'No file', ignored");
    QTest::ignoreMessage(QtWarningMsg, "No files defined for game 'Bad file', ignored");
    QTest::ignoreMessage(QtWarningMsg, "No files defined for game 'Missing file', ignored");

    const QStringList game_dirs { QStringLiteral(":/malformed") };


    providers::SearchContext sctx;
    providers::logiqx::LogiqxProvider provider;
    provider
        .load_with_config(game_dirs)
        .findLists(sctx);

    // TODO: C++17
    QVector<model::Collection*> collections;
    QVector<model::Game*> games;
    std::tie(collections, games) = sctx
        .finalize_lists()
        .consume();

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
    using Rx = QRegularExpression;
    QTest::ignoreMessage(QtInfoMsg, Rx("Logiqx: Found `:.simple.something\\.dat`"));

    const QStringList game_dirs { QStringLiteral(":/simple") };

    providers::SearchContext sctx;
    providers::logiqx::LogiqxProvider provider;
    provider
        .load_with_config(game_dirs)
        .findLists(sctx);

    // TODO: C++17
    QVector<model::Collection*> collections;
    QVector<model::Game*> games;
    std::tie(collections, games) = sctx
        .finalize_lists()
        .consume();

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
