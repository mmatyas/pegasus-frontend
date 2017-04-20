#include "Es2AssetFinder.h"

#include "Model.h"
#include "Utils.h"

#include <QDir>

namespace Es2 {

QString AssetFinder::find(Assets::Type asset_type, const Model::Platform& platform, const Model::Game& game)
{
    Q_ASSERT(!platform.m_short_name.isEmpty());
    Q_ASSERT(!platform.m_rom_dir_path.isEmpty());
    Q_ASSERT(!game.m_rom_basename.isEmpty());

    // check all possible [basedir] + [subdir] + [suffix] + [extension]
    // combination when searching for an asset

    const auto& possible_suffixes = Assets::suffixes[asset_type];
    const auto& possible_exts = Assets::extensions(asset_type);

    const QString es2_subdir = "/downloaded_images/" + platform.m_short_name + "/" + game.m_rom_basename;
    const QVector<QString> possible_base_paths = {
        // portable paths
        platform.m_rom_dir_path + "/media/" + game.m_rom_basename,
        // installation paths
        QDir::homePath() + "/.config/emulationstation" + es2_subdir,
        QDir::homePath() + "/.emulationstation" + es2_subdir,
        "/etc/emulationstation" + es2_subdir,
    };

    // check every combination until there's a match
    for (const auto& base_path : possible_base_paths) {
        for (const auto& asset_suffix : possible_suffixes) {
            for (const auto& ext : possible_exts) {
                if (validFile(base_path + asset_suffix + ext))
                    return base_path + asset_suffix + ext;
            }
        }
    }

    return QString();
}

void AssetFinder::findAll(const Model::Platform& platform, Model::Game& game)
{
    using Type = Assets::Type;

    Q_ASSERT(game.m_assets);
    Model::GameAssets& assets = *game.m_assets;

    // TODO: this should be better as a map
    // TODO: do not overwrite
    assets.m_box_front = find(Type::BOX_FRONT, platform, game);
    assets.m_box_back = find(Type::BOX_BACK, platform, game);
    assets.m_box_spine = find(Type::BOX_SPINE, platform, game);
    assets.m_box_full = find(Type::BOX_FULL, platform, game);
    assets.m_cartridge = find(Type::CARTRIDGE, platform, game);
    assets.m_logo = find(Type::LOGO, platform, game);
    assets.m_marquee = find(Type::MARQUEE, platform, game);
    assets.m_bezel = find(Type::BEZEL, platform, game);
    assets.m_gridicon = find(Type::STEAMGRID, platform, game);
    assets.m_flyer = find(Type::FLYER, platform, game);

    // TODO: support multiple
    assets.m_fanarts << find(Type::FANARTS, platform, game);
    assets.m_screenshots << find(Type::SCREENSHOTS, platform, game);
    assets.m_videos << find(Type::VIDEOS, platform, game);
}

} // namespace Es2
