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

#include "model/gaming/GameAssets.h"


class test_GameAssets : public QObject
{
    Q_OBJECT

private slots:
    void setSingle();
    void appendMulti();
};

void test_GameAssets::setSingle()
{
    modeldata::GameAssets modeldata;
    modeldata.setSingle(AssetType::BOX_FRONT, QUrl::fromLocalFile("/dummy").toString());

    model::GameAssets assets(&modeldata);
    QCOMPARE(assets.boxFront(), QLatin1String("file:///dummy"));
}

void test_GameAssets::appendMulti()
{
    modeldata::GameAssets modeldata;
    modeldata.appendMulti(AssetType::VIDEOS, QUrl::fromLocalFile("/dummy").toString());

    model::GameAssets assets(&modeldata);
    QCOMPARE(assets.videos().count(), 1);
    QCOMPARE(assets.videos().constFirst(), QLatin1String("file:///dummy"));
}


QTEST_MAIN(test_GameAssets)
#include "test_GameAssets.moc"
