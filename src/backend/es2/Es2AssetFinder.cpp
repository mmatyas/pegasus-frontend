#include "Es2AssetFinder.h"

#include "Model.h"
#include "Utils.h"

#include <QDir>
#include <QStringBuilder>


namespace Es2 {

QString AssetFinder::findAsset(Assets::Type asset_type,
                               const Model::Platform& platform,
                               const Model::Game& game)
{
    Q_ASSERT(!platform.m_short_name.isEmpty());
    Q_ASSERT(!platform.m_rom_dir_path.isEmpty());
    Q_ASSERT(!game.m_rom_basename.isEmpty());
    Q_ASSERT(Assets::suffixes.contains(asset_type));

    const auto& possible_suffixes = Assets::suffixes[asset_type];
    const auto& possible_fileexts = Assets::extensions(asset_type);

    // check ES2-specific paths

    static const QString es2_image_dir = QDir::homePath()
                                       % "/.emulationstation/downloaded_images/";

    const QString common_subpath = es2_image_dir
                                 % platform.m_short_name % "/"
                                 % game.m_rom_basename;

    for (const auto& suffix : possible_suffixes) {
        for (const auto& ext : possible_fileexts) {
            const QString path = common_subpath % suffix % ext;
            if (validFile(path))
                return path;
        }
    }

    return QString::null;
}

} // namespace Es2
