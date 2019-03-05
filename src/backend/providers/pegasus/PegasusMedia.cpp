// Pegasus Frontend
// Copyright (C) 2017-2019  Mátyás Mustoha
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


#include "PegasusMedia.h"

#include "PegasusAssets.h"
#include "modeldata/gaming/GameData.h"

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QStringBuilder>


namespace {
AssetType detect_asset_type(const QString& basename, const QString& ext)
{
    const AssetType type = pegasus_assets::str_to_type(basename);
    if (pegasus_assets::allowed_asset_exts(type).contains(ext))
        return type;

    return AssetType::UNKNOWN;
}
} // namespace


namespace providers {
namespace pegasus {

void find_assets(const std::vector<QString>& dir_list, HashMap<size_t, modeldata::Game>& games)
{
    // shortpath: canonical path to dir + extensionless filename
    HashMap<QString, modeldata::Game* const> games_by_shortpath;
    games_by_shortpath.reserve(games.size());
    for (auto& entry : games) {
        modeldata::Game& game = entry.second;

        for (const modeldata::GameFile& gf_entry : qAsConst(game.files)) {
            const QFileInfo& fi = gf_entry.fileinfo;

            QString shortpath = fi.canonicalPath() % '/' % fi.completeBaseName();
            games_by_shortpath.emplace(std::move(shortpath), &game);
        }
    }


    constexpr auto dir_filters = QDir::Files | QDir::Readable | QDir::NoDotAndDotDot;
    constexpr auto dir_flags = QDirIterator::Subdirectories | QDirIterator::FollowSymlinks;

    for (const QString& dir_base : dir_list) {
        const QString media_dir = dir_base + QStringLiteral("/media");

        QDirIterator dir_it(media_dir, dir_filters, dir_flags);
        while (dir_it.hasNext()) {
            dir_it.next();
            const QFileInfo fileinfo = dir_it.fileInfo();

            const QString shortpath = fileinfo.canonicalPath().remove(dir_base.length(), 6); // len of `/media`
            if (!games_by_shortpath.count(shortpath))
                continue;

            const AssetType asset_type = detect_asset_type(fileinfo.completeBaseName(), fileinfo.suffix());
            if (asset_type == AssetType::UNKNOWN)
                continue;

            modeldata::Game* const game = games_by_shortpath[shortpath];
            game->assets.addFileMaybe(asset_type, dir_it.filePath());
        }
    }
}

} // namespace pegasus
} // namespace providers
