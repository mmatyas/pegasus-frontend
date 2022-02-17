// Pegasus Frontend
// Copyright (C) 2017-2020  Mátyás Mustoha
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


#include "MediaProvider.h"

#include "PegasusAssets.h"
#include "model/gaming/Assets.h"
#include "model/gaming/Game.h"
#include "model/gaming/GameFile.h"
#include "types/AssetType.h"
#include "providers/SearchContext.h"
#include "utils/PathTools.h"

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

HashMap<QString, model::Game*> create_lookup_map(const HashMap<QString, model::GameFile*>& games)
{
    HashMap<QString, model::Game*> out;

    // TODO: C++17
    for (const auto& pair : games) {
        const QFileInfo fi(pair.first);
        model::Game* const game_ptr = pair.second->parentGame();

        QString extless_path = ::clean_abs_dir(fi) % QChar('/') % fi.completeBaseName();
        out.emplace(std::move(extless_path), game_ptr);

        // NOTE: the files are not necessarily in the same directory
        QString title_path = ::clean_abs_dir(fi) % QChar('/') % game_ptr->title();
        out.emplace(std::move(title_path), game_ptr);
    }

    return out;
}
} // namespace


namespace providers {
namespace media {

MediaProvider::MediaProvider(QObject* parent)
    : Provider(QLatin1String("pegasus_media"), QStringLiteral("Media"), PROVIDER_FLAG_INTERNAL, parent)
{}

Provider& MediaProvider::run(SearchContext& sctx)
{
    constexpr auto dir_filters = QDir::Files | QDir::NoDotAndDotDot;
    constexpr auto dir_flags = QDirIterator::Subdirectories | QDirIterator::FollowSymlinks;
    constexpr int media_len = 6; // length of `/media`

    const HashMap<QString, model::Game*> lookup_map = create_lookup_map(sctx.current_filepath_to_entry_map());

    for (const QString& dir_base : sctx.pegasus_game_dirs()) {
        const QString media_dir = dir_base + QLatin1String("/media");

        QDirIterator dir_it(media_dir, dir_filters, dir_flags);
        while (dir_it.hasNext()) {
            dir_it.next();
            const QFileInfo fileinfo = dir_it.fileInfo();

            const QString lookup_key = ::clean_abs_dir(fileinfo).remove(dir_base.length(), media_len);
            const auto lookup_it = lookup_map.find(lookup_key);
            if (lookup_it == lookup_map.cend())
                continue;

            const AssetType asset_type = detect_asset_type(fileinfo.completeBaseName(), fileinfo.suffix());
            if (asset_type == AssetType::UNKNOWN)
                continue;

            lookup_it->second->assetsMut().add_file(asset_type, dir_it.filePath());
        }
    }

    return *this;
}

} // namespace media
} // namespace providers
