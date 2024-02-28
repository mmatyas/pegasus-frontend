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


#include "EpicGamesProvider.h"

#include "Log.h"
#include "Paths.h"
#include "providers/ProviderUtils.h"
#include "providers/SearchContext.h"
#include "providers/epicgames/EpicGamesGamelist.h"
#include "utils/CommandTokenizer.h"
#include "utils/StdHelpers.h"

#include <QDir>
#include <QSettings>
#include <QStandardPaths>
#include <QStringBuilder>
#include <QTextStream>


namespace {
QString find_epicgames_datadir()
{
    std::vector<QString> possible_dirs;

    // TODO: Add search for reg keys
    /* const QSettings reg_base(QStringLiteral("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\TODO"), QSettings::NativeFormat); */
    /* const QString reg_value = reg_base.value(QLatin1String("SteamPath")).toString(); */
    /* if (!reg_value.isEmpty()) */
    /*     possible_dirs.emplace_back(reg_value + QChar('/')); */

    // TODO: Remove hardcoded strings if possible
    possible_dirs.emplace_back(QLatin1String("C:\\ProgramData\\Epic\\EpicGamesLauncher\\Data\\Manifests"));

    for (const QString& dir_path : possible_dirs) {
        if (QFileInfo::exists(dir_path))
            return dir_path;
    }

    return {};
}

QString find_epicgames_call()
{
    // TODO: Remove hardcoded strings if possible
    return ::utils::escape_command(QLatin1String("C:\\Program Files (x86)\\Epic Games\\Launcher\\Portal\\Binaries\\Win64\\EpicGamesLauncher.exe"));
}
} // namespace


namespace providers {
namespace epicgames {

EpicGamesProvider::EpicGamesProvider(QObject* parent)
    : Provider(QLatin1String("epicgames"), QStringLiteral("Epic Games"), parent)
{}

Provider& EpicGamesProvider::run(SearchContext& sctx)
{
    const QString epicgamesdir_path = find_epicgames_datadir();
    if (epicgamesdir_path.isEmpty()) {
        Log::info(display_name(), LOGMSG("No installation found"));
        return *this;
    }

    Log::info(display_name(), LOGMSG("Found Epic Games data at `%1`").arg(epicgamesdir_path));

    model::Collection& collection = *sctx.get_or_create_collection(QStringLiteral("Epic Games"));

    const Gamelist gamehelper(display_name());

	const QString epicgames_call = find_epicgames_call();

    HashMap<QString, model::Game*> appid_game_map = gamehelper.find_in(epicgames_call, epicgamesdir_path, collection, sctx);

    Log::info(display_name(), LOGMSG("%1 games found").arg(QString::number(appid_game_map.size())));
    if (appid_game_map.empty())
        return *this;

    return *this;
}

} // namespace epicgames
} // namespace providers
