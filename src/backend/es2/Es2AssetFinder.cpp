#include "Es2AssetFinder.h"

#include "Model.h"
#include "Utils.h"

#include <QDir>
#include <QStringBuilder>


namespace Es2 {

QString AssetFinder::find(Assets::Type asset_type, const Model::Platform& platform, const Model::Game& game)
{
    Q_ASSERT(!platform.m_short_name.isEmpty());
    Q_ASSERT(!platform.m_rom_dir_path.isEmpty());
    Q_ASSERT(!game.m_rom_basename.isEmpty());
    Q_ASSERT(Assets::suffixes.contains(asset_type));

    // check all possible [basedir] + [subdir] + [suffix] + [extension]
    // combination when searching for an asset

    const auto& possible_suffixes = Assets::suffixes[asset_type];
    const auto& possible_fileexts = Assets::extensions(asset_type);

    // check portable paths
    // TODO: move this out of ES2
    static const QLatin1String media_subdir("/media/");
    for (const auto& suffix : possible_suffixes) {
        for (const auto& ext : possible_fileexts) {
            const QString path = platform.m_rom_dir_path
                                 % media_subdir
                                 % game.m_rom_basename
                                 % suffix + ext;
            if (validFile(path))
                return path;
        }
    }

    // check ES2-specific paths

    static const QVector<QString> es2_root_paths = {
        QDir::homePath() % "/.emulationstation/downloaded_images/"
    };

    const QString es2_subpath = platform.m_short_name % "/" % game.m_rom_basename;

    for (const auto& root_path : es2_root_paths) {
        for (const auto& suffix : possible_suffixes) {
            for (const auto& ext : possible_fileexts) {
                const QString path = root_path % es2_subpath % suffix % ext;
                if (validFile(path))
                    return path;
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
