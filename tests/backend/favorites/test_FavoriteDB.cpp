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

#include "QStringHash.h"
#include "configfiles/FavoriteDB.h"
#include "modeldata/gaming/Collection.h"
#include "modeldata/gaming/Game.h"


class test_FavoriteDB : public QObject {
    Q_OBJECT

private slots:
    void write();
    void rewrite_empty();
    void read();
};


void test_FavoriteDB::write()
{
    using GamePtr = QSharedPointer<modeldata::Game>;

    std::vector<modeldata::Collection> list;
    list.emplace_back(modeldata::Collection("coll1"));
    list.back().gamesMut().emplace_back(GamePtr::create(QFileInfo(":/a/b/coll1dummy1")));
    list.back().gamesMut().emplace_back(GamePtr::create(QFileInfo(":/coll1dummy2")));
        list.back().games().back()->is_favorite = true;
    list.emplace_back(modeldata::Collection("coll2"));
    list.back().gamesMut().emplace_back(GamePtr::create(QFileInfo(":/x/y/z/coll2dummy1")));
        list.back().games().back()->is_favorite = true;

    QTemporaryFile tmp_file;
    tmp_file.setAutoRemove(false);
    QVERIFY(tmp_file.open());

    const QString db_path = tmp_file.fileName();
    tmp_file.close();


    FavoriteWriter favorite_db(db_path);

    QSignalSpy spy_start(&favorite_db, &FavoriteWriter::startedWriting);
    QSignalSpy spy_end(&favorite_db, &FavoriteWriter::finishedWriting);
    QVERIFY(spy_start.isValid());
    QVERIFY(spy_end.isValid());

    favorite_db.queueTask(list);
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
    using GamePtr = QSharedPointer<modeldata::Game>;

    std::vector<modeldata::Collection> list;
    list.emplace_back(modeldata::Collection("coll1"));
    list.back().gamesMut().emplace_back(GamePtr::create(QFileInfo(":/a/b/coll1dummy1")));
    list.back().gamesMut().emplace_back(GamePtr::create(QFileInfo(":/coll1dummy2")));
    list.emplace_back(modeldata::Collection("coll2"));
    list.back().gamesMut().emplace_back(GamePtr::create(QFileInfo(":/x/y/z/coll2dummy1")));

    QTemporaryFile tmp_file;
    tmp_file.setAutoRemove(false);
    QVERIFY(tmp_file.open());

    const QString db_path = tmp_file.fileName();
    tmp_file.close();


    FavoriteWriter favorite_db(db_path);
    QSignalSpy spy_end(&favorite_db, &FavoriteWriter::finishedWriting);
    QVERIFY(spy_end.isValid());

    list.at(0).games().at(1)->is_favorite = true;
    favorite_db.queueTask(list);

    list.at(0).games().at(1)->is_favorite = false;
    favorite_db.queueTask(list);
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
    std::unordered_map<QString, QSharedPointer<modeldata::Game>> games;
    const QStringList game_paths {
        QStringLiteral(":/a/b/coll1dummy1"),
        QStringLiteral(":/coll1dummy2"),
        QStringLiteral(":/x/y/z/coll2dummy1"),
    };
    for (const QString& path : game_paths)
        games[path] = QSharedPointer<modeldata::Game>::create(path);

    QTemporaryFile tmp_file;
    tmp_file.setAutoRemove(false);
    QVERIFY(tmp_file.open());
    {
        QTextStream tmp_stream(&tmp_file);
        tmp_stream << QStringLiteral("# Favorite reader test") << endl;
        tmp_stream << game_paths[2] << endl;
        tmp_stream << game_paths[1] << endl;
        tmp_stream << QStringLiteral(":/somethingfake") << endl;
    }
    const QString db_path = tmp_file.fileName();
    tmp_file.close();

    FavoriteReader::readDB(games, db_path);

    QVERIFY(games.size() == 3);
    QVERIFY(games.count(game_paths[0]) && !games.at(game_paths[0])->is_favorite);
    QVERIFY(games.count(game_paths[1]) && games.at(game_paths[1])->is_favorite);
    QVERIFY(games.count(game_paths[2]) && games.at(game_paths[2])->is_favorite);

    QFile::remove(db_path);
}


QTEST_MAIN(test_FavoriteDB)
#include "test_FavoriteDB.moc"
