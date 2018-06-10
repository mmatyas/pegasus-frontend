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

#include "model/Filters.h"
#include "model/gaming/GameList.h"
#include "modeldata/gaming/Collection.h"


class test_GameList : public QObject {
    Q_OBJECT

private slots:
    void empty();
    void nonempty();

    void sortGames();

    void indexChange();
    void indexChange_data();

    void indexIncDecEmpty();
    void indexIncDec();
    void indexIncDec_data();

    void applyFilters();
    void applyFilters_data();

    void letterJump();

private:
    const model::Game* const null_game = static_cast<model::Game*>(nullptr); // Qt 5.7
};

void test_GameList::empty()
{
    std::vector<modeldata::GamePtr> modeldata;
    model::GameList list(modeldata);

    QCOMPARE(list.property("current").value<model::Game*>(), null_game);
    QCOMPARE(list.property("index").toInt(), -1);
    QCOMPARE(list.property("count").toInt(), 0);
    QCOMPARE(list.property("countAll").toInt(), 0);
}

void test_GameList::nonempty()
{
    std::vector<modeldata::GamePtr> modeldata;
    modeldata.emplace_back(modeldata::GamePtr::create(QFileInfo("a")));
    modeldata.emplace_back(modeldata::GamePtr::create(QFileInfo("b")));
    modeldata.emplace_back(modeldata::GamePtr::create(QFileInfo("c")));
    model::GameList list(modeldata);

    QCOMPARE(list.property("current").value<model::Game*>(), list.allGames().first());
    QCOMPARE(list.property("index").toInt(), 0);
    QCOMPARE(list.property("count").toInt(), 3);
    QCOMPARE(list.property("countAll").toInt(), 3);
}

void test_GameList::sortGames()
{
    modeldata::Collection collection("dummy");
    collection.gamesMut().emplace_back(modeldata::GamePtr::create(QFileInfo("bbb")));
    collection.gamesMut().emplace_back(modeldata::GamePtr::create(QFileInfo("aaa")));
    collection.sortGames();
    model::GameList list(collection.games());

    QCOMPARE(list.allGames().front()->property("title").toString(), QLatin1String("aaa"));
    QCOMPARE(list.allGames().back()->property("title").toString(), QLatin1String("bbb"));
}

void test_GameList::indexChange()
{
    std::vector<modeldata::GamePtr> modeldata;
    modeldata.emplace_back(modeldata::GamePtr::create(QFileInfo("a")));
    modeldata.emplace_back(modeldata::GamePtr::create(QFileInfo("b")));
    model::GameList list(modeldata);

    QSignalSpy triggered(&list, &model::GameList::currentChanged);
    QVERIFY(triggered.isValid());
    QVERIFY(triggered.count() == 0);
    QVERIFY(list.property("index").toInt() == 0);


    QFETCH(int, target);
    QFETCH(int, expected);
    if (target != expected)
        QTest::ignoreMessage(QtWarningMsg, QRegularExpression("Invalid game index.*"));

    list.setProperty("index", target);

    QCOMPARE(list.property("index").toInt(), expected);
    QCOMPARE(triggered.count(), expected == 0 ? 0 : 1);
    model::Game* current_ptr = list.property("current").value<model::Game*>();
    if (expected == -1) {
        QCOMPARE(current_ptr, null_game);
    }
    else {
        Q_ASSERT(0 <= expected && expected < list.allGames().count());
        QCOMPARE(current_ptr, list.allGames().at(expected));
    }
}

void test_GameList::indexChange_data()
{
    QTest::addColumn<int>("target");
    QTest::addColumn<int>("expected");

    QTest::newRow("same") << 0 << 0;
    QTest::newRow("different") << 1 << 1;
    QTest::newRow("undefined (-1)") << -1 << -1;
    QTest::newRow("out of range (pos)") << 999 << 0;
    QTest::newRow("out of range (neg)") << -999 << 0;
}

void test_GameList::indexIncDecEmpty()
{
    std::vector<modeldata::GamePtr> modeldata;
    model::GameList list(modeldata);
    QVERIFY(list.property("index").toInt() == -1);

    // increment empty -> stays -1
    QMetaObject::invokeMethod(&list, "incrementIndex", Qt::DirectConnection);;
    QCOMPARE(list.property("index").toInt(), -1);

    // decrement empty -> stays -1
    QMetaObject::invokeMethod(&list, "decrementIndex", Qt::DirectConnection);
    QCOMPARE(list.property("index").toInt(), -1);
}

void test_GameList::indexIncDec_data()
{
    QTest::addColumn<int>("start_idx");
    QTest::addColumn<QString>("metacall");
    QTest::addColumn<int>("expected_idx");

    QTest::newRow("increment regular") << 0 << "incrementIndex" << 1;
    QTest::newRow("increment last") << 1 << "incrementIndex" << 0;
    QTest::newRow("decrement regular") << 1 << "decrementIndex" << 0;
    QTest::newRow("decrement first") << 0 << "decrementIndex" << 1;
    QTest::newRow("increment regular, no wrap") << 0 << "incrementIndexNoWrap" << 1;
    QTest::newRow("increment last, no wrap") << 1 << "incrementIndexNoWrap" << 1;
    QTest::newRow("decrement regular, no wrap") << 1 << "decrementIndexNoWrap" << 0;
    QTest::newRow("decrement first, no wrap") << 0 << "decrementIndexNoWrap" << 0;
}

void test_GameList::indexIncDec()
{
    std::vector<modeldata::GamePtr> modeldata;
    modeldata.emplace_back(modeldata::GamePtr::create(QFileInfo("aaa")));
    modeldata.emplace_back(modeldata::GamePtr::create(QFileInfo("bbb")));
    model::GameList list(modeldata);

    QFETCH(int, start_idx);
    QFETCH(QString, metacall);
    QFETCH(int, expected_idx);

    list.setProperty("index", start_idx);
    QVERIFY(list.property("index").toInt() == start_idx);

    QMetaObject::invokeMethod(&list, metacall.toStdString().c_str(), Qt::DirectConnection);
    QCOMPARE(list.property("index").toInt(), expected_idx);
}

void test_GameList::applyFilters()
{
    std::vector<modeldata::GamePtr> modeldata;
    modeldata.emplace_back(modeldata::GamePtr::create(QFileInfo("game0")));
        modeldata.back()->title = "not-fav, 1P";
        modeldata.back()->is_favorite = false;
        modeldata.back()->player_count = 1;
    modeldata.emplace_back(modeldata::GamePtr::create(QFileInfo("game1")));
        modeldata.back()->title = "not-fav, 2P";
        modeldata.back()->is_favorite = false;
        modeldata.back()->player_count = 2;
    modeldata.emplace_back(modeldata::GamePtr::create(QFileInfo("game2")));
        modeldata.back()->title = "fav, 1P";
        modeldata.back()->is_favorite = true;
        modeldata.back()->player_count = 1;
    modeldata.emplace_back(modeldata::GamePtr::create(QFileInfo("game3")));
        modeldata.back()->title = "My Game";
        modeldata.back()->is_favorite = false;
        modeldata.back()->player_count = 1;
    modeldata.emplace_back(modeldata::GamePtr::create(QFileInfo("game4")));
        modeldata.back()->title = "Another Game";
        modeldata.back()->is_favorite = true;
        modeldata.back()->player_count = 1;

    model::GameList gamelist(modeldata);
    QSignalSpy triggered(&gamelist, &model::GameList::filteredGamesChanged);
    QVERIFY(triggered.isValid());
    QVERIFY(triggered.count() == 0);

    // just to make sure
    QVERIFY(gamelist.property("count").toInt() == 5);
    QVERIFY(gamelist.property("countAll").toInt() == 5);


    QFETCH(model::Filters*, filters);
    QFETCH(int, matching_games_cnt);

    gamelist.applyFilters(*filters);
    QCOMPARE(gamelist.property("count").toInt(), matching_games_cnt);
    QCOMPARE(gamelist.property("countAll").toInt(), 5);

    // if the filter didn't change the list of games, there should be
    // no new trigger -- we only check the count here for simplicity
    if (matching_games_cnt == gamelist.allGames().count())
        QCOMPARE(triggered.count(), 0);
    else
        QCOMPARE(triggered.count(), 1);

    // turning off the filters restores the game count
    filters->setProperty("gameTitle", QString());
    for (model::Filter* const filter : filters->elements())
        filter->setProperty("enabled", false);

    gamelist.applyFilters(*filters);
    QCOMPARE(gamelist.property("count").toInt(), 5);
    QCOMPARE(gamelist.property("countAll").toInt(), 5);
}

void test_GameList::applyFilters_data()
{
    QTest::addColumn<model::Filters*>("filters");
    QTest::addColumn<int>("matching_games_cnt");

    {
        model::Filters* filters = new model::Filters(this);

        QTest::newRow("empty") << filters << 5;
    }
    {
        model::Filters* filters = new model::Filters(this);
        filters->setProperty("gameTitle", "My Game");

        QTest::newRow("full title") << filters << 1;
    }
    {
        model::Filters* filters = new model::Filters(this);
        filters->setProperty("gameTitle", "Game");

        QTest::newRow("partial title") << filters << 2;
    }
    // NOTE: inserting a filter outside the Filters ctor
    // may not update/signal the Qt properties for the change
    {
        model::Filters* filters = new model::Filters(this);
        filters->elementsMut().append(new model::Filter("", filters));
        filters->elements().last()->rulesMut().append(model::FilterRule {
            QLatin1String("favorite"),
            model::FilterRuleType::IS_TRUE,
            QRegularExpression(),
        });
        filters->elements().last()->setProperty("enabled", true);

        QTest::newRow("favorite") << filters << 2;
    }
    {
        model::Filters* filters = new model::Filters(this);

        filters->elementsMut().append(new model::Filter("", filters));
        filters->elements().last()->rulesMut().append(model::FilterRule {
            QLatin1String("title"),
            model::FilterRuleType::CONTAINS,
            QRegularExpression("fav"),
        });
        filters->elements().last()->setProperty("enabled", true);

        filters->elementsMut().append(new model::Filter("", filters));
        filters->elements().last()->rulesMut().append(model::FilterRule {
            QLatin1String("players"),
            model::FilterRuleType::EQUALS,
            QRegularExpression("2"),
        });
        filters->elements().last()->setProperty("enabled", true);

        QTest::newRow("title filter + players") << filters << 1;
    }
}

void test_GameList::letterJump()
{
    std::vector<modeldata::GamePtr> modeldata;
    modeldata.emplace_back(modeldata::GamePtr::create(QFileInfo("Alfa")));
    modeldata.emplace_back(modeldata::GamePtr::create(QFileInfo("Beta")));
    modeldata.emplace_back(modeldata::GamePtr::create(QFileInfo("Gamma")));

    model::GameList list(modeldata);
    QVERIFY(list.property("index").toInt() == 0);

    QMetaObject::invokeMethod(&list, "jumpToLetter", Q_ARG(QString, "g"));
    QVERIFY(list.property("index").toInt() == 2);

    QMetaObject::invokeMethod(&list, "jumpToLetter", Q_ARG(QString, "b"));
    QVERIFY(list.property("index").toInt() == 1);
}


QTEST_MAIN(test_GameList)
#include "test_GameList.moc"
