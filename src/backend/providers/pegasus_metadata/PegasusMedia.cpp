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
#include "model/gaming/Game.h"
#include "types/AssetType.h"

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

HashMap<QString, model::Game* const> create_lookup_map(const HashMap<size_t, model::Game*>& games)
{
    HashMap<QString, model::Game* const> out;

    for (const auto& game_entry : games) {
        model::Game* const game = game_entry.second;

        for (const model::GameFile* const gf_entry : game->filesConst()) {
            const QFileInfo& fi = gf_entry->fileinfo();

            QString extless_path = fi.canonicalPath() % QChar('/') % fi.completeBaseName();
            out.emplace(std::move(extless_path), game);

            // NOTE: the files are not necessarily in the same directory
            //const QString& title = game->title();
            const QString& title = game->title();
            QString title_path = fi.canonicalPath() % QChar('/') % title;
            out.emplace(std::move(title_path), game);
        }
    }

    return out;
}
} // namespace


namespace providers {
namespace pegasus {

void find_assets(const std::vector<QString>& all_dirs, HashMap<size_t, model::Game*>& games)
{
    constexpr auto dir_filters = QDir::Files | QDir::Readable | QDir::NoDotAndDotDot;
    constexpr auto dir_flags = QDirIterator::Subdirectories | QDirIterator::FollowSymlinks;
    constexpr int media_len = 6; // len of `/media`

    const HashMap<QString, model::Game* const> lookup_map = create_lookup_map(games);

    for (const QString& dir_base : all_dirs) {
        const QString media_dir = dir_base + QLatin1String("/media");

        QDirIterator dir_it(media_dir, dir_filters, dir_flags);
        while (dir_it.hasNext()) {
            dir_it.next();
            const QFileInfo fileinfo = dir_it.fileInfo();

            const QString lookup_key = fileinfo.canonicalPath().remove(dir_base.length(), media_len);
            const auto lookup_it = lookup_map.find(lookup_key);
            if (lookup_it == lookup_map.cend())
                continue;

            const AssetType asset_type = detect_asset_type(fileinfo.completeBaseName(), fileinfo.suffix());
            if (asset_type == AssetType::UNKNOWN)
                continue;

            model::Game& game = *lookup_it->second;
            game.assets().add_file(asset_type, dir_it.filePath());
        }
    }
}

} // namespace pegasus
} // namespace providers
