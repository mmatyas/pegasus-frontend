// Pegasus Frontend
// Copyright (C) 2017  Mátyás Mustoha
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
