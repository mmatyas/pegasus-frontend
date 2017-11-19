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

#include "types/Filters.h"
#include "types/Platform.h"


class test_Model : public QObject
{
    Q_OBJECT

private slots:
    void assetsSetSingle();
    void assetsAppendMulti();
};

void test_Model::assetsSetSingle()
{
    Types::GameAssets assets;
    QCOMPARE(assets.boxFront(), QString());

    assets.setSingle(AssetType::BOX_FRONT, QUrl::fromLocalFile("/dummy").toString());
    QCOMPARE(assets.boxFront(), QLatin1String("file:///dummy"));
}

void test_Model::assetsAppendMulti()
{
    Types::GameAssets assets;
    QCOMPARE(assets.videos().count(), 0);

    assets.appendMulti(AssetType::VIDEOS, QUrl::fromLocalFile("/dummy").toString());
    QCOMPARE(assets.videos().count(), 1);
    QCOMPARE(assets.videos().constFirst(), QLatin1String("file:///dummy"));
}


QTEST_MAIN(test_Model)
#include "test_Model.moc"
