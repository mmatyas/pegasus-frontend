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
#include "providers/SearchContext.h"
#include "providers/pegasus_playtime/PlaytimeStats.h"

#include <QSqlDatabase>


namespace {
void create_dummy_data(providers::SearchContext& sctx)
{
    model::Collection& collection_a = *sctx.get_or_create_collection(QStringLiteral("coll1"));
    model::Collection& collection_b = *sctx.get_or_create_collection(QStringLiteral("coll2"));

    model::Game& game_a = *sctx.create_game_for(collection_a);
    sctx.game_add_filepath(game_a, QStringLiteral("dummy1"));

    model::Game& game_b = *sctx.create_game_for(collection_a);
    sctx.game_add_filepath(game_b, QStringLiteral("dummy2"));

    model::Game& game_c = *sctx.create_game_for(collection_b);
    sctx.game_add_filepath(game_c, QStringLiteral(":/x/y/z/coll2dummy1"));
}
} // namespace


class test_Playtime : public QObject {
    Q_OBJECT

private slots:
    void read();
    void write();
    void write_queue();
};

void test_Playtime::read()
{
    const QString db_path = QDir::tempPath() + QStringLiteral("/data.db");
    QFile::remove(db_path);
    QFile::copy(QStringLiteral(":/data.db"), db_path);

    providers::SearchContext sctx;
    create_dummy_data(sctx);
    providers::playtime::PlaytimeStats(db_path).run(sctx);
    const auto [collections, games] = sctx.finalize(this);

    const auto it = std::find_if(games.cbegin(), games.cend(),
        [](const model::Game* const game){ return game->title() == QLatin1String("dummy1"); });
    Q_ASSERT(it != games.cend());
    const model::Game& game = **it;

    QCOMPARE(game.property("playCount").toInt(), 4);
    QCOMPARE(game.property("playTime").toInt(), 35 /*sec*/);
    QCOMPARE(game.property("lastPlayed").toDateTime(), QDateTime::fromSecsSinceEpoch(1531755039));
}

void test_Playtime::write()
{
    QTemporaryFile db_file;
    QVERIFY(db_file.open());

    providers::SearchContext sctx;
    create_dummy_data(sctx);
    providers::playtime::PlaytimeStats playtime(db_file.fileName());
    const auto [collections, games] = sctx.finalize(this);

    QSignalSpy spy_start(&playtime, &providers::playtime::PlaytimeStats::startedWriting);
    QSignalSpy spy_end(&playtime, &providers::playtime::PlaytimeStats::finishedWriting);
    QVERIFY(spy_start.isValid() && spy_end.isValid());

    playtime.onGameLaunched(games.at(0)->filesConst().first());
    playtime.onGameFinished(games.at(0)->filesConst().first());

    QVERIFY(spy_start.count() || spy_start.wait());
    QVERIFY(spy_end.count() || spy_end.wait());
    QCOMPARE(spy_start.count(), 1);
    QCOMPARE(spy_end.count(), 1);

    QCOMPARE(games.at(0)->property("playCount").toInt(), 1);
}

void test_Playtime::write_queue()
{
    QTemporaryFile db_file;
    QVERIFY(db_file.open());

    providers::SearchContext sctx;
    create_dummy_data(sctx);
    providers::playtime::PlaytimeStats playtime(db_file.fileName());
    const auto [collections, games] = sctx.finalize(this);

    QSignalSpy spy_start(&playtime, &providers::playtime::PlaytimeStats::startedWriting);
    QSignalSpy spy_end(&playtime, &providers::playtime::PlaytimeStats::finishedWriting);
    QVERIFY(spy_start.isValid() && spy_end.isValid());


    playtime.onGameLaunched(games.at(0)->filesConst().first());
    playtime.onGameFinished(games.at(0)->filesConst().first());

    playtime.onGameLaunched(games.at(0)->filesConst().first());
    playtime.onGameFinished(games.at(0)->filesConst().first());

    playtime.onGameLaunched(games.at(0)->filesConst().first());
    playtime.onGameFinished(games.at(0)->filesConst().first());


    QVERIFY(spy_start.count() || spy_start.wait());
    QVERIFY(spy_end.count() || spy_end.wait());
    QCOMPARE(spy_start.count(), 1);
    QCOMPARE(spy_end.count(), 1);

#ifndef Q_OS_MACOS
    // FIXME: Flaky results on Mac
    QCOMPARE(games.at(0)->property("playCount").toInt(), 3);
#endif
}


QTEST_MAIN(test_Playtime)
#include "test_Playtime.moc"
