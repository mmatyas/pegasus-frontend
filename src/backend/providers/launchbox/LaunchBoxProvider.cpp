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

#include "LocaleUtils.h"
#include "Log.h"
#include "Paths.h"
#include "providers/SearchContext.h"
#include "providers/launchbox/LaunchBoxAssets.h"
#include "providers/launchbox/LaunchBoxEmulator.h"
#include "providers/launchbox/LaunchBoxEmulatorsXml.h"
#include "providers/launchbox/LaunchBoxGamelistXml.h"
#include "providers/launchbox/LaunchBoxPlatformsXml.h"


namespace {
QString find_installation()
{
    const QString possible_path = paths::homePath() + QStringLiteral("/LaunchBox/");
    if (QFileInfo::exists(possible_path))
        return possible_path;

    return {};
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
            : find_installation();
    }();
    if (lb_dir_path.isEmpty()) {
        Log::info(tr_log("%1: No installation found").arg(display_name()));
        return *this;
    }

    Log::info(tr_log("%1: Looking for installation at `%2`").arg(display_name(), QDir::toNativeSeparators(lb_dir_path)));
    const QDir lb_dir(lb_dir_path);

    const std::vector<QString> platform_names = find_platforms(display_name(), lb_dir);
    if (platform_names.empty()) {
        Log::warning(tr_log("%1: No platforms found").arg(display_name()));
        return *this;
    }

    const HashMap<QString, Emulator> emulators = EmulatorsXml(display_name(), lb_dir).find();
    if (emulators.empty()) {
        Log::warning(tr_log("%1: No emulator settings found").arg(display_name()));
        return *this;
    }

    const float progress_step = 1.f / platform_names.size();
    float progress = 0.f;

    const GamelistXml metahelper(display_name(), lb_dir);
    const Assets assethelper(display_name(), lb_dir_path);
    for (const QString& platform_name : platform_names) {
        const std::vector<model::Game*> games = metahelper.find_games_for(platform_name, emulators, sctx);
        assethelper.find_assets_for(platform_name, games);

        progress += progress_step;
        emit progressChanged(progress);
    }

    return *this;
}

} // namespace launchbox
} // namespace providers
