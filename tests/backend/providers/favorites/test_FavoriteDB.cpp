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

#include "model/gaming/Collection.h"
#include "model/gaming/Game.h"
#include "providers/pegasus_favorites/Favorites.h"
#include "providers/SearchContext.h"


namespace {
void create_dummy_data(providers::SearchContext& sctx)
{
    model::Collection& collection_a = *sctx.get_or_create_collection(QStringLiteral("coll1"));
    model::Collection& collection_b = *sctx.get_or_create_collection(QStringLiteral("coll2"));

    model::Game& game_a = *sctx.create_game_for(collection_a);
    sctx.game_add_filepath(game_a, QStringLiteral(":/a/b/coll1dummy1"));

    model::Game& game_b = *sctx.create_game_for(collection_a);
    sctx.game_add_filepath(game_b, QStringLiteral(":/coll1dummy2"));

    model::Game& game_c = *sctx.create_game_for(collection_b);
    sctx.game_add_filepath(game_c, QStringLiteral(":/x/y/z/coll2dummy1"));

    model::Game& game_d = *sctx.create_game_for(collection_b);
    sctx.game_add_uri(game_d, QStringLiteral("steam:1337"));
}
} // namespace


class test_FavoriteDB : public QObject {
    Q_OBJECT

private slots:
    void write();
    void rewrite_empty();
    void read();
};


void test_FavoriteDB::write()
{
    providers::SearchContext sctx;
    create_dummy_data(sctx);
    sctx.game_by_filepath(QStringLiteral(":/coll1dummy2"))->setFavorite(true);
    sctx.game_by_filepath(QStringLiteral(":/x/y/z/coll2dummy1"))->setFavorite(true);
    sctx.game_by_uri(QStringLiteral("steam:1337"))->setFavorite(true);
    const auto [collections, games] = sctx.finalize(this->thread());

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
    QFile::remove(db_path);

    QCOMPARE(found_items.count(), 3);
    QVERIFY(found_items.contains(":/coll1dummy2"));
    QVERIFY(found_items.contains(":/x/y/z/coll2dummy1"));
    QVERIFY(found_items.contains("steam:1337"));
}

void test_FavoriteDB::rewrite_empty()
{
    providers::SearchContext sctx;
    create_dummy_data(sctx);
    const auto [collections, games] = sctx.finalize(this->thread());

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
    QFile::remove(db_path);

    QCOMPARE(found_items.count(), 0);
}

void test_FavoriteDB::read()
{
    providers::SearchContext sctx;
    create_dummy_data(sctx);

    QTemporaryFile tmp_file;
    tmp_file.setAutoRemove(false);
    QVERIFY(tmp_file.open());
    {
        QTextStream tmp_stream(&tmp_file);
        tmp_stream << QStringLiteral("# Favorite reader test") << endl;
        tmp_stream << QStringLiteral(":/x/y/z/coll2dummy1") << endl;
        tmp_stream << QStringLiteral(":/coll1dummy2") << endl;
        tmp_stream << QStringLiteral(":/somethingfake") << endl;
        tmp_stream << QStringLiteral("steam:1337") << endl;
    }
    const QString db_path = tmp_file.fileName();
    tmp_file.close();

    providers::favorites::Favorites(db_path).run(sctx);
    const auto [collections, games] = sctx.finalize(this->thread());
    QFile::remove(db_path);

    QCOMPARE(games[0]->isFavorite(), true);
    QCOMPARE(games[1]->isFavorite(), false);
    QCOMPARE(games[2]->isFavorite(), true);
    QCOMPARE(games[3]->isFavorite(), true);
}


QTEST_MAIN(test_FavoriteDB)
#include "test_FavoriteDB.moc"
