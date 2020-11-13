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


#include "Es2Provider.h"

#include "LocaleUtils.h"
#include "Log.h"
#include "providers/es2/Es2Games.h"
#include "providers/es2/Es2Metadata.h"
#include "providers/es2/Es2Systems.h"


namespace providers {
namespace es2 {

Es2Provider::Es2Provider(QObject* parent)
    : Provider(QLatin1String("es2"), QStringLiteral("EmulationStation"), parent)
{}

Provider& Es2Provider::run(SearchContext& sctx)
{
    // Find systems
    const std::vector<SystemEntry> systems = find_systems(display_name());
    if (systems.empty())
        return *this;
    Log::info(tr_log("%1: Found %2 systems").arg(display_name(), QString::number(systems.size())));

    const float progress_step = 1.f / (systems.size() * 2);
    float progress = 0.f;

    // Load MAME blacklist, if exists
    const std::vector<QString> mame_blacklist = read_mame_blacklists(display_name());

    // Find games
    for (const SystemEntry& sysentry : systems) {
        const size_t found_games = find_games_for(sysentry, sctx, mame_blacklist);
        Log::info(tr_log("%1: System `%2` provided %3 games")
            .arg(display_name(), sysentry.name, QString::number(found_games)));

        progress += progress_step;
        emit progressChanged(progress);
    }

    // Find assets
    const Metadata metahelper(display_name());
    for (const SystemEntry& sysentry : systems) {
        metahelper.find_metadata_for(sysentry, sctx);

        progress += progress_step;
        emit progressChanged(progress);
    }

    return *this;
}

} // namespace es2
} // namespace providers
