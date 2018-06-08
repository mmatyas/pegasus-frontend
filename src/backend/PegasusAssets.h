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


#pragma once

#include <QFileInfo>
#include <QString>
#include <unordered_map>

enum class AssetType : unsigned char;
namespace modeldata { struct Game; }
namespace modeldata { struct GameAssets; }


namespace pegasus_assets {

struct AssetCheckResult {
    const QString basename;
    const AssetType asset_type;

    bool isValid() const;
};
AssetCheckResult checkFile(const QFileInfo&);
void addAssetToGame(modeldata::GameAssets&, AssetType, const QString&);

void findAssets(const QStringList& asset_dirs,
                const std::unordered_map<QString, QSharedPointer<modeldata::Game>>& games);

} // namespace pegasus_assets
