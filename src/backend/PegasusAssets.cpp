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


#include "PegasusAssets.h"

#include "types/AssetType.h"
#include "utils/HashMap.h"

#include <QString>
#include <QStringList>


namespace pegasus_assets {

AssetType str_to_type(const QString& str)
{
    static const HashMap<QString, const AssetType> map {
        { QStringLiteral("boxfront"), AssetType::BOX_FRONT },
        { QStringLiteral("boxFront"), AssetType::BOX_FRONT },
        { QStringLiteral("box_front"), AssetType::BOX_FRONT },
        { QStringLiteral("boxart2D"), AssetType::BOX_FRONT },
        { QStringLiteral("boxart2d"), AssetType::BOX_FRONT },

        { QStringLiteral("boxback"), AssetType::BOX_BACK },
        { QStringLiteral("boxBack"), AssetType::BOX_BACK },
        { QStringLiteral("box_back"), AssetType::BOX_BACK },

        { QStringLiteral("boxspine"), AssetType::BOX_SPINE },
        { QStringLiteral("boxSpine"), AssetType::BOX_SPINE },
        { QStringLiteral("box_spine"), AssetType::BOX_SPINE },

        { QStringLiteral("boxside"), AssetType::BOX_SPINE },
        { QStringLiteral("boxSide"), AssetType::BOX_SPINE },
        { QStringLiteral("box_side"), AssetType::BOX_SPINE },

        { QStringLiteral("boxfull"), AssetType::BOX_FULL },
        { QStringLiteral("boxFull"), AssetType::BOX_FULL },
        { QStringLiteral("box_full"), AssetType::BOX_FULL },
        { QStringLiteral("box"), AssetType::BOX_FULL },

        { QStringLiteral("cartridge"), AssetType::CARTRIDGE },
        { QStringLiteral("disc"), AssetType::CARTRIDGE },
        { QStringLiteral("cart"), AssetType::CARTRIDGE },
        { QStringLiteral("logo"), AssetType::LOGO },
        { QStringLiteral("wheel"), AssetType::LOGO },
        { QStringLiteral("marquee"), AssetType::ARCADE_MARQUEE },
        { QStringLiteral("bezel"), AssetType::ARCADE_BEZEL },
        { QStringLiteral("screenmarquee"), AssetType::ARCADE_BEZEL },
        { QStringLiteral("border"), AssetType::ARCADE_BEZEL },
        { QStringLiteral("panel"), AssetType::ARCADE_PANEL },

        { QStringLiteral("cabinetleft"), AssetType::ARCADE_CABINET_L },
        { QStringLiteral("cabinetLeft"), AssetType::ARCADE_CABINET_L },
        { QStringLiteral("cabinet_left"), AssetType::ARCADE_CABINET_L },

        { QStringLiteral("cabinetright"), AssetType::ARCADE_CABINET_R },
        { QStringLiteral("cabinetRight"), AssetType::ARCADE_CABINET_R },
        { QStringLiteral("cabinet_right"), AssetType::ARCADE_CABINET_R },

        { QStringLiteral("tile"), AssetType::UI_TILE },
        { QStringLiteral("banner"), AssetType::UI_BANNER },
        { QStringLiteral("steam"), AssetType::UI_STEAMGRID },
        { QStringLiteral("steamgrid"), AssetType::UI_STEAMGRID },
        { QStringLiteral("grid"), AssetType::UI_STEAMGRID },
        { QStringLiteral("poster"), AssetType::POSTER },
        { QStringLiteral("flyer"), AssetType::POSTER },
        { QStringLiteral("background"), AssetType::BACKGROUND },
        { QStringLiteral("music"), AssetType::MUSIC },

        { QStringLiteral("screenshot"), AssetType::SCREENSHOT },
        { QStringLiteral("screenshots"), AssetType::SCREENSHOT },
        { QStringLiteral("video"), AssetType::VIDEO },
        { QStringLiteral("videos"), AssetType::VIDEO },
    };

    const auto it = map.find(str);
    if (it != map.cend())
        return it->second;

    for (const auto& it : map) {
        if (str.startsWith(it.first))
            return it.second;
    }

    return AssetType::UNKNOWN;
}

AssetType ext_to_type(const QString& ext)
{
    static const HashMap<QString, AssetType> map {
        { QStringLiteral("png"), AssetType::BOX_FRONT },
        { QStringLiteral("jpg"), AssetType::BOX_FRONT },
        { QStringLiteral("webp"), AssetType::BOX_FRONT },
        { QStringLiteral("webm"), AssetType::VIDEO },
        { QStringLiteral("mp4"), AssetType::VIDEO },
        { QStringLiteral("avi"), AssetType::VIDEO },
        { QStringLiteral("mp3"), AssetType::MUSIC },
        { QStringLiteral("ogg"), AssetType::MUSIC },
        { QStringLiteral("wav"), AssetType::MUSIC },
    };

    const auto it = map.find(ext);
    if (it != map.cend())
        return it->second;

    return AssetType::UNKNOWN;
}

const QStringList& allowed_asset_exts(AssetType type)
{
    static const QStringList empty_list({});
    static const QStringList image_exts { "png", "jpg", "webp" };
    static const QStringList video_exts { "webm", "mp4", "avi" };
    static const QStringList audio_exts { "mp3", "ogg", "wav" };

    switch (type) {
        case AssetType::UNKNOWN:
            return empty_list;
        case AssetType::VIDEO:
            return video_exts;
        case AssetType::MUSIC:
            return audio_exts;
        default:
            return image_exts;
    }
}

} // namespace pegasus_assets
