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

#include "model/gaming/Collection.h"
#include "model/gaming/Game.h"


class test_Collection : public QObject {
    Q_OBJECT

private slots:
    void names();
    void games();
};

void test_Collection::names()
{
    modeldata::Collection modeldata("myname");
    modeldata.setShortName("abbrev");
    modeldata.launch_cmd = "runner";
    model::Collection collection(std::move(modeldata));
    collection.setGameList({});

    // the properties are read-only and should be called only after the initial setup
    QCOMPARE(collection.property("shortName").toString(), QStringLiteral("abbrev"));
    QCOMPARE(collection.property("name").toString(), QStringLiteral("myname"));
}

void test_Collection::games()
{
    QVector<model::Game*> games = {
        new model::Game(modeldata::Game(QFileInfo("c")), this),
        new model::Game(modeldata::Game(QFileInfo("a")), this),
        new model::Game(modeldata::Game(QFileInfo("b")), this),
    };
    model::Collection collection(modeldata::Collection("test"));
    collection.setGameList(std::move(games));

    // matching count and sorted by title
    QCOMPARE(collection.games().size(), 3);
    QCOMPARE(collection.games().at(0)->title(), "a");
    QCOMPARE(collection.games().at(1)->title(), "b");
    QCOMPARE(collection.games().at(2)->title(), "c");
}


QTEST_MAIN(test_Collection)
#include "test_Collection.moc"
