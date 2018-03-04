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


class test_PegasusProvider : public QObject {
    Q_OBJECT

private slots:
    void find_in_empty_dir();
};

void test_PegasusProvider::find_in_empty_dir()
{
    QHash<QString, Types::Game*> games;
    QHash<QString, Types::Collection*> collections;

    providers::pegasus::PegasusProvider provider;
    provider.find_in_dirs({ QStringLiteral(":/empty") }, games, collections);

    QCOMPARE(games.count(), 0);
    QCOMPARE(collections.count(), 0);
}


QTEST_MAIN(test_PegasusProvider)
#include "test_PegasusProvider.moc"
