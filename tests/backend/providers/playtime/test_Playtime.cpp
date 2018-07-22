// Pegasus Frontend
// Copyright (C) 2018  Mátyás Mustoha
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
#include "providers/pegasus_playtime/PlaytimeStats.h"

#include <QSqlDatabase>

using PlaytimeStats = providers::playtime::PlaytimeStats;


namespace {

void create_dummy_data(QVector<model::Game*>& games,
                       QVector<model::Collection*>& collections,
                       HashMap<QString, model::Game*>& modelgame_map,
                       QObject* parent)
{
    collections = {
        new model::Collection(modeldata::Collection("coll1"), parent),
        new model::Collection(modeldata::Collection("coll2"), parent),
    };
    games = {
        new model::Game(modeldata::Game(QFileInfo("dummy1")), parent),
        new model::Game(modeldata::Game(QFileInfo("dummy2")), parent),
    };
    modelgame_map = {
        { "dummy1", games.at(0) },
        { "dummy2", games.at(1) },
    };

    collections.at(0)->setGameList({ games.at(0) });
    collections.at(1)->setGameList({ games.at(1) });
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
    QVector<model::Game*> games;
    QVector<model::Collection*> collections;
    HashMap<QString, model::Game*> modelgame_map;
    create_dummy_data(games, collections, modelgame_map, this);

    const QString db_path = QDir::tempPath() + QStringLiteral("/data.db");
    QFile::remove(db_path);
    QFile::copy(QStringLiteral(":/data.db"), db_path);


    PlaytimeStats playtime(db_path);

    playtime.findDynamicData(games, collections, modelgame_map);


    QCOMPARE(games.at(0)->data().playcount, 4);
    QCOMPARE(games.at(0)->data().playtime, 35 /*sec*/);
    QCOMPARE(games.at(0)->data().last_played, QDateTime::fromSecsSinceEpoch(1531755039));
}

void test_Playtime::write()
{
    QVector<model::Game*> games;
    QVector<model::Collection*> collections;
    HashMap<QString, model::Game*> modelgame_map;
    create_dummy_data(games, collections, modelgame_map, this);

    QTemporaryFile db_file;
    QVERIFY(db_file.open());


    PlaytimeStats playtime(db_file.fileName());

    QSignalSpy spy_start(&playtime, &providers::playtime::PlaytimeStats::startedWriting);
    QSignalSpy spy_end(&playtime, &providers::playtime::PlaytimeStats::finishedWriting);
    QVERIFY(spy_start.isValid() && spy_end.isValid());

    playtime.onGameLaunched(collections.at(0), games.at(0));
    playtime.onGameFinished(collections.at(0), games.at(0));

    QVERIFY(spy_start.count() || spy_start.wait());
    QVERIFY(spy_end.count() || spy_end.wait());
    QCOMPARE(spy_start.count(), 1);
    QCOMPARE(spy_end.count(), 1);

    QCOMPARE(games.at(0)->property("playCount").toInt(), 1);
}

void test_Playtime::write_queue()
{
    QVector<model::Game*> games;
    QVector<model::Collection*> collections;
    HashMap<QString, model::Game*> modelgame_map;
    create_dummy_data(games, collections, modelgame_map, this);

    QTemporaryFile db_file;
    QVERIFY(db_file.open());


    PlaytimeStats playtime(db_file.fileName());

    QSignalSpy spy_start(&playtime, &providers::playtime::PlaytimeStats::startedWriting);
    QSignalSpy spy_end(&playtime, &providers::playtime::PlaytimeStats::finishedWriting);
    QVERIFY(spy_start.isValid() && spy_end.isValid());


    playtime.onGameLaunched(collections.at(0), games.at(0));
    playtime.onGameFinished(collections.at(0), games.at(0));

    playtime.onGameLaunched(collections.at(0), games.at(0));
    playtime.onGameFinished(collections.at(0), games.at(0));

    playtime.onGameLaunched(collections.at(0), games.at(0));
    playtime.onGameFinished(collections.at(0), games.at(0));


    QVERIFY(spy_start.count() || spy_start.wait());
    QVERIFY(spy_end.count() || spy_end.wait());
    QCOMPARE(spy_start.count(), 1);
    QCOMPARE(spy_end.count(), 1);

    QCOMPARE(games.at(0)->property("playCount").toInt(), 3);
}


QTEST_MAIN(test_Playtime)
#include "test_Playtime.moc"
