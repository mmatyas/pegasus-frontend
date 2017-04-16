#include "Es2Assets.h"

#include "Model.h"
#include "Utils.h"

#include <QDir>


QVector<QString> Es2Assets::possibleSuffixes(AssetType asset_type)
{
    static const QMap<AssetType, QVector<QString>> suffix_map = {
        { AssetType::BOX_FRONT, { "-boxFront", "-box_front", "-boxart2D", "" } },
        { AssetType::BOX_BACK, { "-boxBack", "-box_back" } },
        { AssetType::BOX_SPINE, { "-boxSpine", "-box_spine", "-boxSide", "-box_side" } },
        { AssetType::BOX_FULL, { "-boxFull", "-box_full", "-box" } },
        { AssetType::LOGO, { "-logo", "-wheel" } },
        { AssetType::MARQUEE, { "-marquee" } },
        { AssetType::BEZEL, { "-bezel", "-screenmarquee", "-border" } },
        { AssetType::STEAMGRID, { "-steam", "-steamgrid", "-grid" } },
        { AssetType::FANARTS, { "-fanart", "-art" } },
        { AssetType::FLYERS, { "-flyer" } },
        { AssetType::SCREENSHOTS, { "-screenshot" } },
        { AssetType::VIDEOS, { "-video" } },
    };

    Q_ASSERT(suffix_map.contains(asset_type));
    return suffix_map.value(asset_type);
}

QVector<QString> Es2Assets::possibleExtensions(AssetType asset_type)
{
    static const QVector<QString> image_exts = { ".png", ".jpg" };
    static const QVector<QString> video_exts = { ".webm", ".mp4", ".avi" };

    return (asset_type == AssetType::VIDEOS) ? video_exts : image_exts;
}

QString Es2Assets::find(AssetType asset_type,
                        const Model::Platform* platform,
                        const Model::Game* game)
{
    if (platform->m_short_name.isEmpty() ||
        platform->m_rom_dir_path.isEmpty() ||
        game->m_rom_filename.isEmpty())
        return QString();

    // check all possible [basedir] + [subdir] + [suffix] + [extension]
    // combination when searching for an asset

    const QVector<QString> possible_suffixes = possibleSuffixes(asset_type);
    const QVector<QString> possible_exts = possibleExtensions(asset_type);
    Q_ASSERT(!possible_suffixes.isEmpty());
    Q_ASSERT(!possible_exts.isEmpty());

    // in portable mode, the files are next to the roms under ./media/,
    // but for regular installations, it's under ./downloaded_images/
    const QString es2_subdir = "/downloaded_images/" + platform->m_short_name + "/" + game->m_rom_filename;
    const QVector<QString> possible_base_paths = {
        // portable paths
        platform->m_rom_dir_path + "/media/" + game->m_rom_filename,
        platform->m_rom_dir_path + es2_subdir,
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
