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

#include "AppSettings.h"
#include "LocaleUtils.h"
#include "modeldata/gaming/GameData.h"

#include <QDebug>
#include <QDirIterator>
#include <QStringBuilder>


namespace {
std::vector<QString> get_game_dirs()
{
    std::vector<QString> game_dirs;

    AppSettings::parse_gamedirs([&game_dirs](const QString& line){
        const QFileInfo finfo(line);
        if (finfo.isDir())
            game_dirs.emplace_back(finfo.canonicalFilePath());
    });

    return game_dirs;
}

HashMap<QString, modeldata::Game* const> build_gamepath_db(std::vector<modeldata::Game>& games)
{
    HashMap<QString, modeldata::Game* const> map;

    for (modeldata::Game& game : games) {
        for (const modeldata::GameFile& file_entry : game.files) {
            const QFileInfo& finfo = file_entry.fileinfo;

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
    : Provider(parent)
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
        { AssetType::SCREENSHOTS, QStringLiteral("screenshot") },
        { AssetType::SCREENSHOTS, QStringLiteral("screenshottitle") },
        { AssetType::UI_STEAMGRID, QStringLiteral("steamgrid") },
        { AssetType::VIDEOS, QStringLiteral("videos") },
    }
    , m_media_dirs {
        QStringLiteral("/skraper/"),
        QStringLiteral("/media/"),
    }
{}

void SkraperAssetsProvider::findStaticData(SearchContext& sctx)
{
    qInfo().noquote() << tr_log("Skraper: Looking for assets...");
    unsigned found_assets_cnt = 0;

    const std::vector<QString> game_dirs = get_game_dirs();
    const HashMap<QString, modeldata::Game* const> extless_path_to_game = build_gamepath_db(sctx.games);

    constexpr auto dir_filters = QDir::Files | QDir::Readable | QDir::NoDotAndDotDot;
    constexpr auto dir_flags = QDirIterator::Subdirectories | QDirIterator::FollowSymlinks;


    for (const QString& game_dir : game_dirs) {
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

                    modeldata::Game* const game = extless_path_to_game.at(game_path);
                    game->assets.addFileMaybe(asset_dir.asset_type, dir_it.filePath());
                    found_assets_cnt++;
                }
            }
        }
    }

    qInfo().noquote() << tr_log("Skraper: %1 assets found").arg(found_assets_cnt);
}

} // namespace skraper
} // namespace providers
