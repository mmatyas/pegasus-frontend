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

#include "FavoriteDB.h"


class test_FavoriteDB : public QObject {
    Q_OBJECT

private slots:
    void write();
    void read();
};


void test_FavoriteDB::write()
{
    types::CollectionList list;
    QTest::ignoreMessage(QtInfoMsg, QRegularExpression("\\d+ games found"));

    // TODO: implement addPlatform
    list.elementsMut().append(new types::Collection("coll1"));
    list.elementsMut().last()->gameListMut().addGame(":/a/b/coll1dummy1");
    list.elementsMut().last()->gameListMut().addGame(":/coll1dummy2");
    list.elementsMut().append(new types::Collection("coll2"));
    list.elementsMut().last()->gameListMut().addGame(":/x/y/z/coll2dummy1");
    list.onScanComplete();

    QVERIFY(list.elements().count() == 2);
    QVERIFY(list.elements().at(0)->gameList().allCount() == 2);
    QVERIFY(list.elements().at(1)->gameList().allCount() == 1);

    list.elements().at(0)->gameList().allGames().at(1)->m_favorite = true;
    list.elements().at(1)->gameList().allGames().at(0)->m_favorite = true;

    QTemporaryFile tmp_file;
    tmp_file.setAutoRemove(false);
    QVERIFY(tmp_file.open());

    const QString db_path = tmp_file.fileName();
    tmp_file.close();

    FavoriteDB favorite_db(db_path);

    QSignalSpy spy_start(&favorite_db, &FavoriteDB::startedWriting);
    QSignalSpy spy_end(&favorite_db, &FavoriteDB::finishedWriting);
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


void test_FavoriteDB::read()
{
    QHash<QString, types::Game*> games;
    const QStringList game_paths {
        QStringLiteral(":/a/b/coll1dummy1"),
        QStringLiteral(":/coll1dummy2"),
        QStringLiteral(":/x/y/z/coll2dummy1"),
    };
    for (const QString& path : game_paths)
        games[path] = new types::Game(path);

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

    QCOMPARE(games.count(), 3);
    QVERIFY(games.contains(game_paths[0]) && !games.value(game_paths[0])->m_favorite);
    QVERIFY(games.contains(game_paths[1]) && games.value(game_paths[1])->m_favorite);
    QVERIFY(games.contains(game_paths[2]) && games.value(game_paths[2])->m_favorite);

    QFile::remove(db_path);
    for (const auto& game : qAsConst(games))
        delete game;
}


QTEST_MAIN(test_FavoriteDB)
#include "test_FavoriteDB.moc"
