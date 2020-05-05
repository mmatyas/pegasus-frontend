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


#include "SkraperAssetsProvider.h"

#include "LocaleUtils.h"
#include "model/gaming/Game.h"

#include <QDebug>
#include <QDirIterator>
#include <QStringBuilder>


namespace {
HashMap<QString, model::Game* const> build_gamepath_db(HashMap<size_t, model::Game*>& games)
{
    HashMap<QString, model::Game* const> map;

    for (auto& entry : games) {
        model::Game& game = *entry.second;
        for (const model::GameFile* const file_ptr : game.filesConst()) {
            const QFileInfo& finfo = file_ptr->fileinfo();

            QString path = finfo.canonicalPath() % '/' % finfo.completeBaseName();
            map.emplace(std::move(path), &game);
        }
    }

    return map;
}
} // namespace


namespace providers {
namespace skraper {

SkraperAssetsProvider::SkraperDir::SkraperDir(AssetType type, QString dir)
    : asset_type(type)
    , dir_name(std::move(dir))
{}


SkraperAssetsProvider::SkraperAssetsProvider(QObject* parent)
    : Provider(QLatin1String("skraper"), QStringLiteral("Skraper Assets"), PROVIDES_ASSETS, parent)
    , m_asset_dirs {
        // NOTE: The entries are ordered by priority
        { AssetType::ARCADE_MARQUEE, QStringLiteral("screenmarquee") },
        { AssetType::ARCADE_MARQUEE, QStringLiteral("screenmarqueesmall") },
        { AssetType::BACKGROUND, QStringLiteral("fanart") },
        { AssetType::BOX_BACK, QStringLiteral("box2dback") },
        { AssetType::BOX_FRONT, QStringLiteral("box2dfront") },
        { AssetType::BOX_FRONT, QStringLiteral("supporttexture") },
        { AssetType::BOX_FULL, QStringLiteral("boxtexture") },
        { AssetType::BOX_SPINE, QStringLiteral("box2dside") },
        { AssetType::CARTRIDGE, QStringLiteral("support") },
        { AssetType::LOGO, QStringLiteral("wheel") },
        { AssetType::LOGO, QStringLiteral("wheelcarbon") },
        { AssetType::LOGO, QStringLiteral("wheelsteel") },
        { AssetType::SCREENSHOT, QStringLiteral("screenshot") },
        { AssetType::SCREENSHOT, QStringLiteral("screenshottitle") },
        { AssetType::UI_STEAMGRID, QStringLiteral("steamgrid") },
        { AssetType::VIDEO, QStringLiteral("videos") },
    }
    , m_media_dirs {
        QStringLiteral("/skraper/"),
        QStringLiteral("/media/"),
    }
{}

void SkraperAssetsProvider::findStaticData(SearchContext& sctx)
{
    unsigned found_assets_cnt = 0;

    const HashMap<QString, model::Game* const> extless_path_to_game = build_gamepath_db(sctx.games);

    constexpr auto dir_filters = QDir::Files | QDir::Readable | QDir::NoDotAndDotDot;
    constexpr auto dir_flags = QDirIterator::Subdirectories | QDirIterator::FollowSymlinks;


    for (const QString& game_dir : sctx.game_root_dirs) {
        for (const QString& media_dir : m_media_dirs) {
            const QString game_media_dir = game_dir % media_dir;
            if (!QFileInfo::exists(game_media_dir))
                continue;

            for (const SkraperDir& asset_dir : m_asset_dirs) {
                const QString search_dir = game_media_dir % asset_dir.dir_name;
                const int subpath_len = media_dir.length() + asset_dir.dir_name.length();

                QDirIterator dir_it(search_dir, dir_filters, dir_flags);
                while (dir_it.hasNext()) {
                    dir_it.next();
                    const QFileInfo finfo = dir_it.fileInfo();

                    const QString game_path = finfo.canonicalPath().remove(game_dir.length(), subpath_len)
                                            % '/' % finfo.completeBaseName();
                    if (!extless_path_to_game.count(game_path))
                        continue;

                    model::Game* const game = extless_path_to_game.at(game_path);
                    game->assets().add_file(asset_dir.asset_type, dir_it.filePath());
                    found_assets_cnt++;
                }
            }
        }
    }

    qInfo().noquote() << tr_log("%1: %2 assets found").arg(name(), QString::number(found_assets_cnt));
}

} // namespace skraper
} // namespace providers
