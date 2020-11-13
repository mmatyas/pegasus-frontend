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

#include "model/gaming/Assets.h"


class test_GameAssets : public QObject
{
    Q_OBJECT

private slots:
    void setSingle();
    void appendMulti();
};

void test_GameAssets::setSingle()
{
    model::Assets assets(this);
    assets.add_uri(AssetType::BOX_FRONT, QUrl::fromLocalFile("/dummy").toString());
    QCOMPARE(assets.property("boxFront").toString(), QLatin1String("file:///dummy"));
}

void test_GameAssets::appendMulti()
{
    model::Assets assets(this);
    assets.add_uri(AssetType::VIDEO, QUrl::fromLocalFile("/dummy1").toString());
    assets.add_uri(AssetType::VIDEO, QUrl::fromLocalFile("/dummy2").toString());

    QCOMPARE(assets.property("videoList").toStringList().count(), 2);
    QCOMPARE(assets.property("videoList").toStringList().constFirst(), QLatin1String("file:///dummy1"));
    QCOMPARE(assets.property("videoList").toStringList().constLast(), QLatin1String("file:///dummy2"));
}


QTEST_MAIN(test_GameAssets)
#include "test_GameAssets.moc"
