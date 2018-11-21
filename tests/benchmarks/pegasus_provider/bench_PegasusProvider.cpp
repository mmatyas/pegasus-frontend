// Pegasus Frontend
// Copyright (C) 2017-2018  Mátyás Mustoha
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

#include "providers/pegasus/PegasusProvider.h"
#include "modeldata/gaming/Collection.h"
#include "modeldata/gaming/Game.h"

#include <QString>


class bench_PegasusProvider : public QObject {
    Q_OBJECT

private slots:
    void find_in_empty_dir();
    void find_in_filled_dir();
};

void bench_PegasusProvider::find_in_empty_dir()
{
    HashMap<QString, modeldata::Game> games;
    HashMap<QString, modeldata::Collection> collections;
    HashMap<QString, std::vector<QString>> collection_childs;

    providers::pegasus::PegasusProvider provider({QStringLiteral(":/empty")});

    QBENCHMARK {
        provider.findLists(games, collections, collection_childs);
    }
}

void bench_PegasusProvider::find_in_filled_dir()
{
    HashMap<QString, modeldata::Game> games;
    HashMap<QString, modeldata::Collection> collections;
    HashMap<QString, std::vector<QString>> collection_childs;

    providers::pegasus::PegasusProvider provider({QStringLiteral(":/filled")});

    QBENCHMARK {
        QTest::ignoreMessage(QtInfoMsg, "Collections: found `:/filled/collections.txt`");
        provider.findLists(games, collections, collection_childs);
    }
}


QTEST_MAIN(bench_PegasusProvider)
#include "bench_PegasusProvider.moc"
