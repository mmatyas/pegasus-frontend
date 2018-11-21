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


#include "GameAssets.h"

#include <QUrl>

#include "types/AssetType.h"


namespace {
bool asset_is_single(AssetType type)
{
    return type != AssetType::SCREENSHOTS && type != AssetType::VIDEOS;
}
} // namespace


namespace modeldata {

GameAssets::GameAssets() = default;

void GameAssets::addFileMaybe(AssetType key, QString path)
{
    QString url = QUrl::fromLocalFile(path).toString();
    addUrlMaybe(key, std::move(url));
}

void GameAssets::addUrlMaybe(AssetType key, QString url)
{
    const bool is_single = asset_is_single(key);

    if (is_single && single(key).isEmpty()) {
        setSingle(key, std::move(url));
    }
    else if (!is_single && !multi(key).contains(url)) {
        appendMulti(key, std::move(url));
    }
}

void GameAssets::setSingle(AssetType key, QString value)
{
    Q_ASSERT(asset_is_single(key));
    m_single_assets[key] = std::move(value);
}

void GameAssets::appendMulti(AssetType key, QString value)
{
    Q_ASSERT(!asset_is_single(key));
    m_multi_assets[key].append(std::move(value));
}

} // namespace modeldata
