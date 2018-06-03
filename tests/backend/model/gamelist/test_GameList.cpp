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


class test_GameList : public QObject {
    Q_OBJECT

private slots:
    void empty();

    void addGame();
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
    model::GameList list;

    QCOMPARE(list.property("current").value<model::Game*>(), null_game);
    QCOMPARE(list.property("index").toInt(), -1);
    QCOMPARE(list.property("count").toInt(), 0);
    QCOMPARE(list.property("countAll").toInt(), 0);
}

void test_GameList::addGame()
{
    model::GameList list;

    list.addGame("a");
    list.addGame("b");
    list.addGame("c");
    list.lockGameList();

    QCOMPARE(list.property("current").value<model::Game*>(), list.allGames().first());
    QCOMPARE(list.property("index").toInt(), 0);
    QCOMPARE(list.property("count").toInt(), 3);
    QCOMPARE(list.property("countAll").toInt(), 3);
}

void test_GameList::sortGames()
{
    model::GameList gamelist;

    gamelist.addGame("bbb");
    gamelist.addGame("aaa");
    gamelist.sortGames();
    gamelist.lockGameList();

    QCOMPARE(gamelist.allGames().first()->m_fileinfo.filePath(), QLatin1String("aaa"));
    QCOMPARE(gamelist.allGames().last()->m_fileinfo.filePath(), QLatin1String("bbb"));
}

void test_GameList::indexChange()
{
    model::GameList list;
    QSignalSpy triggered(&list, &model::GameList::currentChanged);
    QVERIFY(triggered.isValid());

    list.addGame("a");
    list.addGame("b");
    list.lockGameList();

    QVERIFY(triggered.count() == 1);
    QVERIFY(list.index() == 0);


    QFETCH(int, target);
    QFETCH(int, expected);
    if (target != expected)
        QTest::ignoreMessage(QtWarningMsg, QRegularExpression("Invalid game index.*"));

    list.setProperty("index", target);

    QCOMPARE(list.property("index").toInt(), expected);
    QCOMPARE(triggered.count(), expected == 0 ? 1 : 2);
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
    model::GameList list;
    QVERIFY(list.index() == -1);

    // increment empty -> stays -1
    list.incrementIndex();
    QCOMPARE(list.index(), -1);

    // decrement empty -> stays -1
    list.decrementIndex();
    QCOMPARE(list.index(), -1);
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
    model::GameList list;
    list.addGame("aaa");
    list.addGame("bbb");
    list.lockGameList();

    QFETCH(int, start_idx);
    QFETCH(QString, metacall);
    QFETCH(int, expected_idx);

    list.setIndex(start_idx);
    QVERIFY(list.index() == start_idx);

    QMetaObject::invokeMethod(&list, metacall.toStdString().c_str(), Qt::DirectConnection);
    QCOMPARE(list.index(), expected_idx);
}

void test_GameList::applyFilters()
{
    model::GameList gamelist;
    QSignalSpy triggered(&gamelist, &model::GameList::filteredGamesChanged);
    QVERIFY(triggered.isValid());

    gamelist.addGame("game0");
        gamelist.allGames().last()->m_title = "not-fav, 1P";
        gamelist.allGames().last()->m_favorite = false;
        gamelist.allGames().last()->m_players = 1;
    gamelist.addGame("game1");
        gamelist.allGames().last()->m_title = "not-fav, 2P";
        gamelist.allGames().last()->m_favorite = false;
        gamelist.allGames().last()->m_players = 2;
    gamelist.addGame("game2");
        gamelist.allGames().last()->m_title = "fav, 1P";
        gamelist.allGames().last()->m_favorite = true;
        gamelist.allGames().last()->m_players = 1;
    gamelist.addGame("game3");
        gamelist.allGames().last()->m_title = "My Game";
        gamelist.allGames().last()->m_favorite = false;
        gamelist.allGames().last()->m_players = 1;
    gamelist.addGame("game4");
        gamelist.allGames().last()->m_title = "Another Game";
        gamelist.allGames().last()->m_favorite = true;
        gamelist.allGames().last()->m_players = 1;

    QVERIFY(triggered.count() == 0);
    gamelist.lockGameList();
    QVERIFY(triggered.count() == 1);

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
        QCOMPARE(triggered.count(), 1);
    else
        QCOMPARE(triggered.count(), 2);

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
    model::GameList list;
    list.addGame("Alfa");
    list.addGame("Beta");
    list.addGame("Gamma");
    list.lockGameList();
    QVERIFY(list.index() == 0);

    list.jumpToLetter("g");
    QVERIFY(list.index() == 2);

    list.jumpToLetter("b");
    QVERIFY(list.index() == 1);
}


QTEST_MAIN(test_GameList)
#include "test_GameList.moc"
