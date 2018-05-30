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

#include "types/gaming/Game.h"


class test_Game : public QObject {
    Q_OBJECT

private slots:
    void developers();
    void publishers();
    void genres();
    void release();
};

void testStrAndList(model::Game& game,
                    std::function<void(const QString&)> fn_add,
                    const char* str_name, const char* list_name)
{
    QVERIFY(game.property(str_name).toString().isEmpty());
    QVERIFY(game.property(list_name).toStringList().isEmpty());

    // empty
    fn_add({});
    QVERIFY(game.property(str_name).toString().isEmpty());
    QVERIFY(game.property(list_name).toStringList().isEmpty());

    // first
    fn_add("test1");
    QCOMPARE(game.property(str_name).toString(), QStringLiteral("test1"));
    QCOMPARE(game.property(list_name).toStringList(), {"test1"});

    // second
    fn_add("test2");
    QCOMPARE(game.property(str_name).toString(), QStringLiteral("test1, test2"));
    QCOMPARE(game.property(list_name).toStringList(), QStringList({"test1", "test2"}));

    // third
    fn_add("test3");
    QCOMPARE(game.property(str_name).toString(), QStringLiteral("test1, test2, test3"));
    QCOMPARE(game.property(list_name).toStringList(), QStringList({"test1", "test2", "test3"}));
}

void test_Game::developers()
{
    model::Game game({});
    auto fn = [&game](const QString& val){ game.addDeveloper(val); };
    testStrAndList(game, fn, "developer", "developerList");
}

void test_Game::publishers()
{
    model::Game game({});
    auto fn = [&game](const QString& val){ game.addPublisher(val); };
    testStrAndList(game, fn, "publisher", "publisherList");
}

void test_Game::genres()
{
    model::Game game({});
    auto fn = [&game](const QString& val){ game.addGenre(val); };
    testStrAndList(game, fn, "genre", "genreList");
}

void test_Game::release()
{
    model::Game game({});
    game.setRelease(QDate(1999,1,2));
    QCOMPARE(game.property("year").toInt(), 1999);
    QCOMPARE(game.property("month").toInt(), 1);
    QCOMPARE(game.property("day").toInt(), 2);
}


QTEST_MAIN(test_Game)
#include "test_Game.moc"
