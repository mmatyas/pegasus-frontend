// Pegasus Frontend
// Copyright (C) 2017  Mátyás Mustoha
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

#include "model/gaming/Game.h"


class test_Game : public QObject {
    Q_OBJECT

private slots:
    void developers();
    void publishers();
    void genres();
    void release();

    void launch();
};

void testStrAndList(modeldata::Game& modeldata,
                    std::function<void(const QString&)> fn_add,
                    const char* str_name, const char* list_name)
{
    model::Game game(&modeldata);

    QVERIFY(game.property(str_name).toString().isEmpty());
    QVERIFY(game.property(list_name).toStringList().isEmpty());

    fn_add("test1");
    fn_add("test2");
    fn_add("test3");

    QCOMPARE(game.property(str_name).toString(), QStringLiteral("test1, test2, test3"));
    QCOMPARE(game.property(list_name).toStringList(), QStringList({"test1", "test2", "test3"}));
}

void test_Game::developers()
{
    modeldata::Game game({});
    auto fn = [&game](const QString& val){ game.developers.append(val); };
    testStrAndList(game, fn, "developer", "developerList");
}

void test_Game::publishers()
{
    modeldata::Game game({});
    auto fn = [&game](const QString& val){ game.publishers.append(val); };
    testStrAndList(game, fn, "publisher", "publisherList");
}

void test_Game::genres()
{
    modeldata::Game game({});
    auto fn = [&game](const QString& val){ game.genres.append(val); };
    testStrAndList(game, fn, "genre", "genreList");
}

void test_Game::release()
{
    modeldata::Game modeldata({});
    modeldata.release_date = QDate(1999,1,2);

    model::Game game(&modeldata);
    QCOMPARE(game.property("releaseYear").toInt(), 1999);
    QCOMPARE(game.property("releaseMonth").toInt(), 1);
    QCOMPARE(game.property("releaseDay").toInt(), 2);
}

void test_Game::launch()
{
    modeldata::Game modeldata({});
    model::Game game(&modeldata);

    QSignalSpy spy_launch(&game, &model::Game::launchRequested);
    QVERIFY(spy_launch.isValid());

    QMetaObject::invokeMethod(&game, "launch");
    QVERIFY(spy_launch.count() == 1 || spy_launch.wait());
}


QTEST_MAIN(test_Game)
#include "test_Game.moc"
