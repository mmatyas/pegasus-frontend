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


#include "LaunchBoxAssets.h"

#include "model/gaming/Assets.h"
#include "model/gaming/Game.h"

#include <QDirIterator>


namespace {
HashMap<QString, model::Game*> build_escaped_title_map(const std::vector<model::Game*>& games)
{
    const QRegularExpression rx_invalid(QStringLiteral(R"([<>:"\/\\|?*'])"));
    const QString underscore(QLatin1Char('_'));

    HashMap<QString, model::Game*> out;
    for (model::Game* const game_ptr : games) {
        QString title = game_ptr->title();
        title.replace(rx_invalid, underscore);
        out.emplace(std::move(title), game_ptr);
    }

    return out;
}
} // namespace


namespace providers {
namespace launchbox {

Assets::Assets(QString log_tag, QString lb_root_path)
    : m_log_tag(std::move(log_tag))
    , m_lb_root_path(std::move(lb_root_path))
    , m_dir_list {
        { QStringLiteral("Box - Front"), AssetType::BOX_FRONT },
        { QStringLiteral("Box - Front - Reconstructed"), AssetType::BOX_FRONT },
        { QStringLiteral("Fanart - Box - Front"), AssetType::BOX_FRONT },

        { QStringLiteral("Box - Back"), AssetType::BOX_BACK },
        { QStringLiteral("Box - Back - Reconstructed"), AssetType::BOX_BACK },
        { QStringLiteral("Fanart - Box - Back"), AssetType::BOX_BACK },

        { QStringLiteral("Arcade - Marquee"), AssetType::ARCADE_MARQUEE },
        { QStringLiteral("Banner"), AssetType::ARCADE_MARQUEE },

        { QStringLiteral("Cart - Front"), AssetType::CARTRIDGE },
        { QStringLiteral("Disc"), AssetType::CARTRIDGE },
        { QStringLiteral("Fanart - Cart - Front"), AssetType::CARTRIDGE },
        { QStringLiteral("Fanart - Disc"), AssetType::CARTRIDGE },

        { QStringLiteral("Screenshot - Gameplay"), AssetType::SCREENSHOT },
        { QStringLiteral("Screenshot - Game Select"), AssetType::SCREENSHOT },
        { QStringLiteral("Screenshot - Game Title"), AssetType::TITLESCREEN },
        { QStringLiteral("Screenshot - Game Over"), AssetType::SCREENSHOT },
        { QStringLiteral("Screenshot - High Scores"), AssetType::SCREENSHOT },

        { QStringLiteral("Advertisement Flyer - Front"), AssetType::POSTER },
        { QStringLiteral("Arcade - Control Panel"), AssetType::ARCADE_PANEL },
        { QStringLiteral("Clear Logo"), AssetType::LOGO },
        { QStringLiteral("Fanart - Background"), AssetType::BACKGROUND },
        { QStringLiteral("Steam Banner"), AssetType::UI_STEAMGRID },
    }
    , rx_number_suffix(QStringLiteral(R"(-[0-9]{2}$)"))
{}

void Assets::find_assets_for(const QString& platform_name, const std::vector<model::Game*>& games) const
{
    const HashMap<QString, model::Game*> esctitle_to_game_map = build_escaped_title_map(games);

    const QString images_root = m_lb_root_path % QLatin1String("Images/") % platform_name % QLatin1Char('/');
    // TODO: C++17
    for (const auto& assetdir_pair : m_dir_list) {
        const QString assetdir_path = images_root + assetdir_pair.first;
        const AssetType assetdir_type = assetdir_pair.second;
        find_assets_in(assetdir_path, assetdir_type, esctitle_to_game_map);
    }

    const QString music_root = m_lb_root_path % QLatin1String("Music/") % platform_name % QLatin1Char('/');
    find_assets_in(music_root, AssetType::MUSIC, esctitle_to_game_map);

    const QString video_root = m_lb_root_path % QLatin1String("Videos/") % platform_name % QLatin1Char('/');
    find_assets_in(video_root, AssetType::VIDEO, esctitle_to_game_map);
}

void Assets::find_assets_in(
    const QString& asset_dir,
    const AssetType asset_type,
    const HashMap<QString, model::Game*>& title_to_game_map) const
{
    constexpr auto FIND_ONLY_FILES = QDir::Files | QDir::Readable | QDir::NoDotAndDotDot;
    constexpr auto ITER_RECURSIVE = QDirIterator::Subdirectories;

    QDirIterator file_it(asset_dir, FIND_ONLY_FILES, ITER_RECURSIVE);
    while (file_it.hasNext()) {
        QString path = file_it.next();

        const QString basename = file_it.fileInfo().completeBaseName();
        auto it = title_to_game_map.find(basename);
        if (it != title_to_game_map.cend())
            it->second->assetsMut().add_file(asset_type, path);

        const bool has_number_suffix = rx_number_suffix.match(basename).hasMatch();
        if (!has_number_suffix)
            continue;

        const QString game_title = basename.left(basename.length() - 3); // gamename "-xx" .ext
        it = title_to_game_map.find(game_title);
        if (it != title_to_game_map.cend())
            it->second->assetsMut().add_file(asset_type, std::move(path));
    }
}

} // namespace launchbox
} // namespace providers
