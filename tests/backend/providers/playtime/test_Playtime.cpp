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
#include "providers/pegasus_playtime/PlaytimeStats.h"

#include <QSqlDatabase>

using PlaytimeStats = providers::playtime::PlaytimeStats;


namespace {

model::Game* create_game(QString path, QObject* parent)
{
    QFileInfo fi(std::move(path));
    QString name = model::pretty_filename(fi);
    auto game = new model::Game(name, parent);
    auto file = new model::GameFile(std::move(fi), game);
    game->setFiles({ file });
    return game;
}

void create_dummy_data(QVector<model::Collection*>& collections,
                       QVector<model::Game*>& games,
                       HashMap<QString, model::GameFile*>& path_map,
                       QObject* parent)
{
    collections = {
        new model::Collection("coll1", parent),
        new model::Collection("coll2", parent),
    };
    games = {
        create_game("dummy1", parent),
        create_game("dummy2", parent),
    };
    path_map = {
        { "dummy1", *games.at(0)->filesConst().cbegin() },
        { "dummy2", *games.at(1)->filesConst().cbegin() },
    };

    collections.at(0)->setGames({ games.at(0) });
    collections.at(1)->setGames({ games.at(1) });
    games.at(0)->setCollections({ collections.at(0) });
    games.at(1)->setCollections({ collections.at(1) });
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
    QVector<model::Collection*> collections;
    QVector<model::Game*> games;
    HashMap<QString, model::GameFile*> path_map;
    create_dummy_data(collections, games, path_map, this);

    const QString db_path = QDir::tempPath() + QStringLiteral("/data.db");
    QFile::remove(db_path);
    QFile::copy(QStringLiteral(":/data.db"), db_path);


    PlaytimeStats playtime;
    playtime.load_with_dbpath(db_path);
    playtime.findDynamicData(collections, games, path_map);

    QCOMPARE(games.at(0)->playCount(), 4);
    QCOMPARE(games.at(0)->playTime(), 35 /*sec*/);
    QCOMPARE(games.at(0)->lastPlayed(), QDateTime::fromSecsSinceEpoch(1531755039));
}

void test_Playtime::write()
{
    QVector<model::Collection*> collections;
    QVector<model::Game*> games;
    HashMap<QString, model::GameFile*> path_map;
    create_dummy_data(collections, games, path_map, this);

    QTemporaryFile db_file;
    QVERIFY(db_file.open());


    PlaytimeStats playtime;
    playtime.load_with_dbpath(db_file.fileName());

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
    QVector<model::Collection*> collections;
    QVector<model::Game*> games;
    HashMap<QString, model::GameFile*> path_map;
    create_dummy_data(collections, games, path_map, this);

    QTemporaryFile db_file;
    QVERIFY(db_file.open());


    PlaytimeStats playtime;
    playtime.load_with_dbpath(db_file.fileName());

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

    QCOMPARE(games.at(0)->property("playCount").toInt(), 3);
}


QTEST_MAIN(test_Playtime)
#include "test_Playtime.moc"
