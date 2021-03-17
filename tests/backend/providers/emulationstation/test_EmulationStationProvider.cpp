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
#include "model/gaming/Assets.h"
#include "providers/SearchContext.h"
#include "providers/es2/Es2Provider.h"


class test_EmulationStationProvider : public QObject {
    Q_OBJECT

private slots:
    void initTestCase() {
        Log::init_qttest();
    }

    void empty();
    void basic();
    void gamelist();
};


void test_EmulationStationProvider::empty()
{
    QTest::ignoreMessage(QtInfoMsg, "EmulationStation: No installation found");

    providers::SearchContext sctx;
    providers::es2::Es2Provider()
        .setOption(QStringLiteral("installdir"), QStringLiteral(":/empty/es"))
        .run(sctx);
    const auto [collections, games] = sctx.finalize(this);

    QCOMPARE(collections.size(), 0);
    QCOMPARE(games.size(), 0);
}


void test_EmulationStationProvider::basic()
{
    QTest::ignoreMessage(QtInfoMsg, "EmulationStation: Found `:/basic/es/es_systems.cfg`");
    QTest::ignoreMessage(QtInfoMsg, "EmulationStation: Found 1 systems");
    QTest::ignoreMessage(QtInfoMsg, "EmulationStation: System `My System 1` provided 1 games");
    QTest::ignoreMessage(QtWarningMsg, "EmulationStation: No gamelist file found for system `MySys1`");

    providers::SearchContext sctx;
    providers::es2::Es2Provider()
        .setOption(QStringLiteral("installdir"), QStringLiteral(":/basic/es"))
        .run(sctx);
    const auto [collections, games] = sctx.finalize(this);

    QCOMPARE(collections.size(), 1);
    QCOMPARE(games.size(), 1);

    QCOMPARE(collections.first()->name(), QStringLiteral("My System 1"));
    QCOMPARE(collections.first()->shortName(), QStringLiteral("mysys1"));
}


void test_EmulationStationProvider::gamelist()
{
    QTest::ignoreMessage(QtInfoMsg, "EmulationStation: Found `:/gamelist/es/es_systems.cfg`");
    QTest::ignoreMessage(QtInfoMsg, "EmulationStation: Found 1 systems");
    QTest::ignoreMessage(QtInfoMsg, "EmulationStation: Found `:/gamelist/mysys1/gamelist.xml`");
    QTest::ignoreMessage(QtInfoMsg, "EmulationStation: System `My System 1` provided 2 games");

    providers::SearchContext sctx;
    providers::es2::Es2Provider()
        .setOption(QStringLiteral("installdir"), QStringLiteral(":/gamelist/es"))
        .run(sctx);
    const auto [collections, games] = sctx.finalize(this);

    QCOMPARE(collections.size(), 1);
    QCOMPARE(games.size(), 2);

    QCOMPARE(collections.first()->name(), QStringLiteral("My System 1"));
    QCOMPARE(collections.first()->shortName(), QStringLiteral("mysys1"));

    model::Game* game = games.constFirst();
    QCOMPARE(game->title(), QStringLiteral("Game 1"));
    QCOMPARE(game->description(), QStringLiteral("Some Description"));
    QCOMPARE(game->developerStr(), QStringLiteral("Some Developer"));
    QCOMPARE(game->publisherStr(), QStringLiteral("Some Publisher"));
    QCOMPARE(game->genreStr(), QStringLiteral("Some Genre"));
    QCOMPARE(game->playerCount(), 4);
    QCOMPARE(game->rating(), 0.6f);
    QCOMPARE(game->playCount(), 10);
    QCOMPARE(game->lastPlayed(), QDateTime::fromString(QStringLiteral("2000-01-02T102030"), Qt::ISODate));
    QCOMPARE(game->releaseDate(), QDate(1999, 1, 2));

    QCOMPARE(game->assets().boxFront(), QStringLiteral("file::/gamelist/abs_image1.png"));
    QCOMPARE(game->assets().video(), QStringLiteral("file::/gamelist/abs_image2.png"));
    QCOMPARE(game->assets().marquee(), QStringLiteral("file::/gamelist/img/abs_image3.png"));

    game = games.constLast();
    QCOMPARE(game->title(), QStringLiteral("Game 2"));
    QCOMPARE(game->playerCount(), 5);

    QCOMPARE(game->assets().boxFront(), QStringLiteral("file::/gamelist/mysys1/local_image1.png"));
    QCOMPARE(game->assets().video(), QStringLiteral("file::/gamelist/mysys1/local_image2.png"));
    QCOMPARE(game->assets().marquee(), QStringLiteral("file::/gamelist/mysys1/img/local_image3.png"));
}


QTEST_MAIN(test_EmulationStationProvider)
#include "test_EmulationStationProvider.moc"
