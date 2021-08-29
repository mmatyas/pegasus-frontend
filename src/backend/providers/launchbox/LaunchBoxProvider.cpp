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


#include "LaunchBoxProvider.h"

#include "Log.h"
#include "Paths.h"
#include "providers/launchbox/LaunchBoxAssets.h"
#include "providers/launchbox/LaunchBoxEmulatorsXml.h"
#include "providers/launchbox/LaunchBoxGamelistXml.h"
#include "providers/launchbox/LaunchBoxPlatformsXml.h"
#include "providers/launchbox/LaunchBoxXml.h"


namespace {
QString default_installation()
{
    return paths::homePath() + QStringLiteral("/LaunchBox/");
}
} // namespace


namespace providers {
namespace launchbox {

LaunchboxProvider::LaunchboxProvider(QObject* parent)
    : Provider(QLatin1String("launchbox"), QStringLiteral("LaunchBox"), parent)
{}

Provider& LaunchboxProvider::run(providers::SearchContext& sctx)
{
    const QString lb_dir_path = [this]{
        const auto option_it = options().find(QStringLiteral("installdir"));
        return (option_it != options().cend())
            ? QDir::cleanPath(option_it->second.front()) + QLatin1Char('/')
            : default_installation();
    }();
    if (lb_dir_path.isEmpty() || !QFileInfo::exists(lb_dir_path)) {
        Log::info(display_name(), LOGMSG("No installation found"));
        return *this;
    }

    Log::info(display_name(), LOGMSG("Looking for installation at `%1`").arg(QDir::toNativeSeparators(lb_dir_path)));
    const QDir lb_dir(lb_dir_path);

    const std::vector<Platform> platforms = find_platforms(display_name(), lb_dir);
    if (platforms.empty()) {
        Log::warning(display_name(), LOGMSG("No platforms found"));
        return *this;
    }

    const HashMap<QString, Emulator> emulators = EmulatorsXml(display_name(), lb_dir).find();
    // NOTE: It's okay to not have any emulators

    const float progress_step = 1.f / platforms.size() / 2.f;
    float progress = 0.f;

    const GamelistXml metahelper(display_name(), lb_dir);
    const Assets assethelper(display_name(), lb_dir_path);
    for (const Platform& platform : platforms) {
        const std::vector<model::Game*> games = metahelper.find_games_for(platform, emulators, sctx);
        progress += progress_step;
        emit progressChanged(progress);

        assethelper.find_assets_for(platform.name, games);
        progress += progress_step;
        emit progressChanged(progress);
    }

    return *this;
}

} // namespace launchbox
} // namespace providers
