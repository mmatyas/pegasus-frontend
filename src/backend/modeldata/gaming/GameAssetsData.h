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


#pragma once

#include "types/AssetType.h"
#include "utils/HashMap.h"

#include <QString>
#include <QStringList>


namespace modeldata {

struct GameAssets {
    explicit GameAssets();

    // NOTE: a value for an asset type may not exist at the time of calling
    const QString& single(AssetType type) { return m_single_assets[type]; }
    const QStringList& multi(AssetType type) { return m_multi_assets[type]; }

    void addFileMaybe(AssetType, QString);
    void addUrlMaybe(AssetType, QString);
    void setSingle(AssetType, QString);
    void appendMulti(AssetType, QString);


    GameAssets(const GameAssets&) = delete;
    GameAssets& operator=(const GameAssets&) = delete;
    GameAssets(GameAssets&&) = default;
    GameAssets& operator=(GameAssets&&) = default;

private:
    HashMap<AssetType, QString> m_single_assets;
    HashMap<AssetType, QStringList> m_multi_assets;
};

} // namespace modeldata
