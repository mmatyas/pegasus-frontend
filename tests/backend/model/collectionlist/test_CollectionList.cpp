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
#include "model/gaming/Collection.h"
#include "model/gaming/CollectionList.h"


class test_CollectionList : public QObject {
    Q_OBJECT

private slots:
    void empty();
    void addPlatform();
};

void test_CollectionList::empty()
{
    model::CollectionList list;
    QCOMPARE(list.property("count").toInt(), 0);
}

void test_CollectionList::addPlatform()
{
    QVector<model::Collection*> collections = {
        new model::Collection(modeldata::Collection("coll1")),
        new model::Collection(modeldata::Collection("coll2")),
    };
    QVector<model::Game*> games = {
        new model::Game(modeldata::Game(QFileInfo("dummy1"))),
        new model::Game(modeldata::Game(QFileInfo("dummy2"))),
    };
    collections.at(0)->setGameList({ games.at(0) });
    collections.at(1)->setGameList({ games.at(1) });

    model::CollectionList list;
    list.setModelData(std::move(collections));
    QCOMPARE(list.property("count").toInt(), 2);
}


QTEST_MAIN(test_CollectionList)
#include "test_CollectionList.moc"
