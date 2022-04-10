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

#include <array>


class test_Collection : public QObject {
    Q_OBJECT

private slots:
    void names();
    void games();
    void sorting();
};

void test_Collection::names()
{
    model::Collection collection("myname");
    collection.setShortName("AbbReV");
    collection.setCommonLaunchCmd("runner");

    // the properties are read-only and should be called only after the initial setup
    QCOMPARE(collection.property("shortName").toString(), QStringLiteral("abbrev"));
    QCOMPARE(collection.property("name").toString(), QStringLiteral("myname"));
}

void test_Collection::games()
{
    std::vector<model::Game*> games {
        new model::Game("a", this),
        new model::Game("b", this),
        new model::Game("c", this),
    };
    const auto collection = new model::Collection("test", this);
    collection->setGames(std::move(games));

    // matching count and sorted by title
    QCOMPARE(collection->gameList()->entries().size(), 3);
    QCOMPARE(collection->gameList()->entries().at(0)->property("title").toString(), QStringLiteral("a"));
    QCOMPARE(collection->gameList()->entries().at(1)->property("title").toString(), QStringLiteral("b"));
    QCOMPARE(collection->gameList()->entries().at(2)->property("title").toString(), QStringLiteral("c"));
}

void test_Collection::sorting()
{
    const std::array<std::pair<QString, QString>, 4> name_pairs {
        std::make_pair("Collection I", "Collection 1"),
        std::make_pair("Collection IX", "Collection 9"),
        std::make_pair("Collection IV", "Collection 4"),
        std::make_pair("Collection 8", QString()), // intentionally missing custom sort
    };

    std::vector<model::Collection*> collections;
    for (const auto& pair : name_pairs) {
        auto* const coll_ptr = new model::Collection(pair.first);
        collections.emplace_back(coll_ptr);

        if (!pair.second.isEmpty())
            coll_ptr->setSortBy(pair.second);
    }

    std::sort(collections.begin(), collections.end(), model::sort_collections);
    QCOMPARE(collections.at(0)->name(), QStringLiteral("Collection I"));
    QCOMPARE(collections.at(1)->name(), QStringLiteral("Collection IV"));
    QCOMPARE(collections.at(2)->name(), QStringLiteral("Collection 8"));
    QCOMPARE(collections.at(3)->name(), QStringLiteral("Collection IX"));
}


QTEST_MAIN(test_Collection)
#include "test_Collection.moc"
