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
    std::unordered_map<QString, QSharedPointer<modeldata::Game>> games;
    std::unordered_map<QString, modeldata::Collection> collections;
    providers::pegasus::PegasusProvider provider;
    provider.add_game_dir(QStringLiteral(":/empty"));

    QBENCHMARK {
        provider.find(games, collections);
    }
}

void bench_PegasusProvider::find_in_filled_dir()
{
    std::unordered_map<QString, QSharedPointer<modeldata::Game>> games;
    std::unordered_map<QString, modeldata::Collection> collections;
    providers::pegasus::PegasusProvider provider;
    provider.add_game_dir(QStringLiteral(":/filled"));

    QBENCHMARK {
        QTest::ignoreMessage(QtInfoMsg, "Found `:/filled/collections.txt`");
        provider.find(games, collections);
    }
}


QTEST_MAIN(bench_PegasusProvider)
#include "bench_PegasusProvider.moc"
