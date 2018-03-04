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

#include "types/Collection.h"
#include "types/Game.h"
#include "providers/pegasus/PegasusProvider.h"
#include <QHash>
#include <QString>


class test_PegasusProvider : public QObject {
    Q_OBJECT

private slots:
    void find_in_empty_dir();
    void find_in_filled_dir();
};

void test_PegasusProvider::find_in_empty_dir()
{
    QHash<QString, Types::Game*> games;
    QHash<QString, Types::Collection*> collections;

    providers::pegasus::PegasusProvider provider;
    provider.find_in_dirs({ QStringLiteral(":/empty") }, games, collections);

    QCOMPARE(collections.count(), 0);
    QCOMPARE(games.count(), 0);
}

void test_PegasusProvider::find_in_filled_dir()
{
    QHash<QString, Types::Game*> games;
    QHash<QString, Types::Collection*> collections;

    providers::pegasus::PegasusProvider provider;
    provider.find_in_dirs({ QStringLiteral(":/filled") }, games, collections);

    // finds the correct collections
    QCOMPARE(collections.count(), 3);
    QVERIFY(collections[QStringLiteral("mygames")] != nullptr);
    QVERIFY(collections[QStringLiteral("faves")] != nullptr);
    QVERIFY(collections[QStringLiteral("multi")] != nullptr);

    // finds the correct amount of games
    QCOMPARE(games.count(), 7);
    QCOMPARE(collections[QStringLiteral("mygames")]->gameList().allCount(), 7);
    QCOMPARE(collections[QStringLiteral("faves")]->gameList().allCount(), 2);
    QCOMPARE(collections[QStringLiteral("multi")]->gameList().allCount(), 1);

    // finds the correct files for the collections
    const QStringList mygames_paths {
        { ":/filled/mygame1.ext" },
        { ":/filled/mygame2.ext" },
        { ":/filled/mygame3.ext" },
        { ":/filled/favgame1.ext" },
        { ":/filled/favgame2.ext" },
        { ":/filled/9999-in-1.ext" },
        { ":/filled/subdir/game_in_subdir.ext" },
    };
    const QStringList faves_paths {
        { ":/filled/favgame1.ext" },
        { ":/filled/favgame2.ext" },
    };
    const QStringList multi_paths {
        { ":/filled/9999-in-1.ext" },
    };
    for (const Types::Game* const game : collections[QStringLiteral("mygames")]->gameList().allGames()) {
        QVERIFY(game != nullptr);
        QCOMPARE(mygames_paths.count(game->m_fileinfo.filePath()), 1);
    }
    for (const Types::Game* const game : collections[QStringLiteral("faves")]->gameList().allGames()) {
        QVERIFY(game != nullptr);
        QCOMPARE(faves_paths.count(game->m_fileinfo.filePath()), 1);
    }
    for (const Types::Game* const game : collections[QStringLiteral("multi")]->gameList().allGames()) {
        QVERIFY(game != nullptr);
        QCOMPARE(multi_paths.count(game->m_fileinfo.filePath()), 1);
    }

    // finds the correct games in general
    const QStringList all_game_paths = mygames_paths;
    for (const Types::Game* const game : games.values()) {
        QVERIFY(game != nullptr);
        QCOMPARE(all_game_paths.count(game->m_fileinfo.filePath()), 1);
    }
}


QTEST_MAIN(test_PegasusProvider)
#include "test_PegasusProvider.moc"
