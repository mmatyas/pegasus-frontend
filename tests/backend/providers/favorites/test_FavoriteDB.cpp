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
#include "providers/pegasus_favorites/Favorites.h"
#include "utils/HashMap.h"


class test_FavoriteDB : public QObject {
    Q_OBJECT

private slots:
    void write();
    void rewrite_empty();
    void read();
};


void test_FavoriteDB::write()
{
    QVector<model::Collection*> collections = {
        new model::Collection(modeldata::Collection("coll1"), this),
        new model::Collection(modeldata::Collection("coll2"), this),
    };
    QVector<model::Game*> games = {
        new model::Game(modeldata::Game(QFileInfo(":/a/b/coll1dummy1")), this),
        new model::Game(modeldata::Game(QFileInfo(":/coll1dummy2")), this),
        new model::Game(modeldata::Game(QFileInfo(":/x/y/z/coll2dummy1")), this),
    };

    games.at(1)->setFavorite(true);
    games.at(2)->setFavorite(true);

    collections.at(0)->setGameList({ games.at(0), games.at(1) });
    collections.at(1)->setGameList({ games.at(2) });

    QTemporaryFile tmp_file;
    tmp_file.setAutoRemove(false);
    QVERIFY(tmp_file.open());

    const QString db_path = tmp_file.fileName();
    tmp_file.close();


    providers::favorites::Favorites favorite_db(db_path);

    QSignalSpy spy_start(&favorite_db, &providers::favorites::Favorites::startedWriting);
    QSignalSpy spy_end(&favorite_db, &providers::favorites::Favorites::finishedWriting);
    QVERIFY(spy_start.isValid());
    QVERIFY(spy_end.isValid());

    favorite_db.onGameFavoriteChanged(games);

    QVERIFY(spy_start.count() || spy_start.wait());
    QVERIFY(spy_end.count() || spy_end.wait());
    QCOMPARE(spy_start.count(), 1);
    QCOMPARE(spy_end.count(), 1);


    QFile db_file(db_path);
    QVERIFY(db_file.open(QFile::ReadOnly | QFile::Text));

    QTextStream db_stream(&db_file);
    QStringList found_items;
    QString line;
    while (db_stream.readLineInto(&line)) {
        if (!line.startsWith('#'))
            found_items << line;
    }

    QCOMPARE(found_items.count(), 2);
    QVERIFY(found_items.contains(":/coll1dummy2"));
    QVERIFY(found_items.contains(":/x/y/z/coll2dummy1"));

    QFile::remove(db_path);
}

void test_FavoriteDB::rewrite_empty()
{
    QVector<model::Collection*> collections = {
        new model::Collection(modeldata::Collection("coll1"), this),
        new model::Collection(modeldata::Collection("coll2"), this),
    };
    QVector<model::Game*> games = {
        new model::Game(modeldata::Game(QFileInfo(":/a/b/coll1dummy1")), this),
        new model::Game(modeldata::Game(QFileInfo(":/coll1dummy2")), this),
        new model::Game(modeldata::Game(QFileInfo(":/x/y/z/coll2dummy1")), this),
    };
    collections.at(0)->setGameList({ games.at(0), games.at(1) });
    collections.at(1)->setGameList({ games.at(2) });

    QTemporaryFile tmp_file;
    tmp_file.setAutoRemove(false);
    QVERIFY(tmp_file.open());

    const QString db_path = tmp_file.fileName();
    tmp_file.close();


    providers::favorites::Favorites favorite_db(db_path);
    QSignalSpy spy_end(&favorite_db, &providers::favorites::Favorites::finishedWriting);
    QVERIFY(spy_end.isValid());

    games.at(1)->setFavorite(true);
    favorite_db.onGameFavoriteChanged(games);

    games.at(1)->setFavorite(false);
    favorite_db.onGameFavoriteChanged(games);

    QVERIFY(spy_end.count() == 2 || spy_end.wait());


    QFile db_file(db_path);
    QVERIFY(db_file.open(QFile::ReadOnly | QFile::Text));

    QTextStream db_stream(&db_file);
    QStringList found_items;
    QString line;
    while (db_stream.readLineInto(&line)) {
        if (!line.startsWith('#'))
            found_items << line;
    }

    QCOMPARE(found_items.count(), 0);
    QFile::remove(db_path);
}

void test_FavoriteDB::read()
{
    QVector<model::Game*> games = {
        new model::Game(modeldata::Game(QFileInfo(":/a/b/coll1dummy1")), this),
        new model::Game(modeldata::Game(QFileInfo(":/coll1dummy2")), this),
        new model::Game(modeldata::Game(QFileInfo(":/x/y/z/coll2dummy1")), this),
    };

    QTemporaryFile tmp_file;
    tmp_file.setAutoRemove(false);
    QVERIFY(tmp_file.open());
    {
        QTextStream tmp_stream(&tmp_file);
        tmp_stream << QStringLiteral("# Favorite reader test") << endl;
        tmp_stream << games[2]->filesConst().first()->data().fileinfo.canonicalFilePath() << endl;
        tmp_stream << games[1]->filesConst().first()->data().fileinfo.canonicalFilePath() << endl;
        tmp_stream << QStringLiteral(":/somethingfake") << endl;
    }
    const QString db_path = tmp_file.fileName();
    tmp_file.close();

    providers::favorites::Favorites favorite_db(db_path);

    HashMap<QString, model::GameFile*> path_map;
    for (const model::Game* const game : games) {
        model::GameFile* const gamefile = game->filesConst().first();
        QString path = gamefile->data().fileinfo.canonicalFilePath();
        QVERIFY(!path.isEmpty());
        path_map.emplace(std::move(path), gamefile);
    }

    favorite_db.findDynamicData({}, games, path_map);

    QVERIFY(!games[0]->data().is_favorite);
    QVERIFY(games[1]->data().is_favorite);
    QVERIFY(games[2]->data().is_favorite);

    QFile::remove(db_path);
}


QTEST_MAIN(test_FavoriteDB)
#include "test_FavoriteDB.moc"
