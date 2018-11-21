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

#include "providers/Provider.h"
#include "types/AssetType.h"
#include "utils/MoveOnly.h"

#include <array>
#include <vector>


namespace providers {
namespace skraper {

class SkraperAssetsProvider : public Provider {
    Q_OBJECT

public:
    explicit SkraperAssetsProvider(QObject* parent = nullptr);

    void findStaticData(HashMap<QString, modeldata::Game>&,
                        const HashMap<QString, modeldata::Collection>&,
                        const HashMap<QString, std::vector<QString>>&) final;

private:
    struct SkraperDir {
        const AssetType asset_type;
        const QString dir_name;

        SkraperDir(AssetType, QString);
    };
    const std::vector<SkraperDir> m_asset_dirs;
    const std::array<QString, 2> m_media_dirs;
};

} // namespace skraper
} // namespace providers
