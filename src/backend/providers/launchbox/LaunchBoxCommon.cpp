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


#include "LaunchBoxCommon.h"

#include "LocaleUtils.h"

#include <QDebug>
#include <QDir>


namespace providers {
namespace launchbox {

Literals::Literals()
    : gamefield_map({
        { QStringLiteral("ID"), GameField::ID },
        { QStringLiteral("ApplicationPath"), GameField::PATH },
        { QStringLiteral("Title"), GameField::TITLE },
        { QStringLiteral("Developer"), GameField::DEVELOPER },
        { QStringLiteral("Publisher"), GameField::PUBLISHER },
        { QStringLiteral("ReleaseDate"), GameField::RELEASE },
        { QStringLiteral("Notes"), GameField::NOTES },
        { QStringLiteral("PlayMode"), GameField::PLAYMODE },
        { QStringLiteral("Genre"), GameField::GENRE },
        { QStringLiteral("CommunityStarRating"), GameField::STARS },
        { QStringLiteral("Emulator"), GameField::EMULATOR_ID },
        { QStringLiteral("CommandLine"), GameField::EMULATOR_PARAMS },
        { QStringLiteral("Platform"), GameField::EMULATOR_PLATFORM },
        { QStringLiteral("VideoPath"), GameField::ASSETPATH_VIDEO },
        { QStringLiteral("MusicPath"), GameField::ASSETPATH_MUSIC },
    })
    , addiappfield_map({
        { QStringLiteral("Id"), AdditionalAppField::ID },
        { QStringLiteral("ApplicationPath"), AdditionalAppField::PATH },
        { QStringLiteral("GameID"), AdditionalAppField::GAME_ID },
        { QStringLiteral("Name"), AdditionalAppField::NAME },
    })
    , assetdir_map({ // ordered by priority
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

        { QStringLiteral("Screenshot - Gameplay"), AssetType::SCREENSHOTS },
        { QStringLiteral("Screenshot - Game Select"), AssetType::SCREENSHOTS },
        { QStringLiteral("Screenshot - Game Title"), AssetType::SCREENSHOTS },
        { QStringLiteral("Screenshot - Game Over"), AssetType::SCREENSHOTS },
        { QStringLiteral("Screenshot - High Scores"), AssetType::SCREENSHOTS },

        { QStringLiteral("Advertisement Flyer - Front"), AssetType::POSTER },
        { QStringLiteral("Arcade - Control Panel"), AssetType::ARCADE_PANEL },
        { QStringLiteral("Clear Logo"), AssetType::LOGO },
        { QStringLiteral("Fanart - Background"), AssetType::BACKGROUND },
        { QStringLiteral("Steam Banner"), AssetType::UI_STEAMGRID },
    })
{}


void log_xml_warning(const QXmlStreamReader& xml, const QString& xml_rel_path, const QString& msg)
{
    qWarning().noquote()
        << QStringLiteral("LaunchBox: %1:%2: %3")
           .arg(QDir::toNativeSeparators(xml_rel_path), QString::number(xml.lineNumber()), msg);
}

void check_lb_root_node(QXmlStreamReader& xml, const QString& xml_rel_path)
{
    if (!xml.readNextStartElement() || xml.name() != QLatin1String("LaunchBox")) {
        xml.raiseError(tr_log("`%1` does not have a `<LaunchBox>` root node, file ignored")
            .arg(QDir::toNativeSeparators(xml_rel_path)));
    }
}

} // namespace launchbox
} // namespace providers
