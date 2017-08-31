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


#include "Assets.h"

#include "Utils.h"

#include <QStringBuilder>


const QVector<AssetType> Assets::singleTypes = {
    AssetType::BOX_FRONT,
    AssetType::BOX_BACK,
    AssetType::BOX_SPINE,
    AssetType::BOX_FULL,
    AssetType::CARTRIDGE,
    AssetType::LOGO,
    AssetType::MARQUEE,
    AssetType::BEZEL,
    AssetType::STEAMGRID,
    AssetType::FLYER,
    AssetType::MUSIC,
};
const QVector<AssetType> Assets::multiTypes = {
    AssetType::FANARTS,
    AssetType::SCREENSHOTS,
    AssetType::VIDEOS,
};

const QHash<AssetType, QVector<QString>> Assets::suffixes = {
    { AssetType::BOX_FRONT, { "-boxFront", "-box_front", "-boxart2D", "" } },
    { AssetType::BOX_BACK, { "-boxBack", "-box_back" } },
    { AssetType::BOX_SPINE, { "-boxSpine", "-box_spine", "-boxSide", "-box_side" } },
    { AssetType::BOX_FULL, { "-boxFull", "-box_full", "-box" } },
    { AssetType::CARTRIDGE, { "-cartridge", "-cart", "-disc" } },
    { AssetType::LOGO, { "-logo", "-wheel" } },
    { AssetType::MARQUEE, { "-marquee" } },
    { AssetType::BEZEL, { "-bezel", "-screenmarquee", "-border" } },
    { AssetType::STEAMGRID, { "-steam", "-steamgrid", "-grid" } },
    { AssetType::FLYER, { "-flyer" } },
    { AssetType::MUSIC, { "-music", "" } },
    // multi
    { AssetType::FANARTS, { "-fanart", "-art" } },
    { AssetType::SCREENSHOTS, { "-screenshot" } },
    { AssetType::VIDEOS, { "-video", "" } },
};

namespace {

#ifdef Q_PROCESSOR_ARM
// prefer opaque images on embedded systems
const QVector<QString> image_exts = { ".jpg", ".png" };
#else
const QVector<QString> image_exts = { ".png", ".jpg" };
#endif

const QVector<QString> video_exts = { ".webm", ".mp4", ".avi" };
const QVector<QString> audio_exts = { ".mp3", ".ogg", ".wav" };

} // namespace

const QVector<QString>& Assets::extensions(AssetType key)
{
   switch (key) {
       case AssetType::VIDEOS:
           return video_exts;
       case AssetType::MUSIC:
           return audio_exts;
       default:
           return image_exts;
   }
}

QString Assets::findFirst(AssetType asset_type, const QString& path_base)
{
    const auto& possible_suffixes = Assets::suffixes[asset_type];
    const auto& possible_fileexts = Assets::extensions(asset_type);

    for (const auto& suffix : possible_suffixes) {
        for (const auto& ext : possible_fileexts) {
            const QString path = path_base % suffix % ext;
            if (validPath(path))
                return path;
        }
    }

    return QString();
}

QStringList Assets::findAll(AssetType asset_type, const QString& path_base)
{
    const auto& possible_suffixes = Assets::suffixes[asset_type];
    const auto& possible_fileexts = Assets::extensions(asset_type);

    QStringList results;

    for (const auto& suffix : possible_suffixes) {
        for (const auto& ext : possible_fileexts) {
            const QString path = path_base % suffix % ext;
            if (validPath(path))
                results.append(path);
        }
    }

    return results;
}
