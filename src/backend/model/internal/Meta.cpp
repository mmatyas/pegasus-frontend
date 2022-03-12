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


#include "Meta.h"

#include "Log.h"
#include "Paths.h"


namespace model {

const QString Meta::m_git_revision(QStringLiteral(GIT_REVISION));
const QString Meta::m_git_date(QStringLiteral(GIT_DATE));

Meta::Meta(const backend::CliArgs& args, QObject* parent)
    : QObject(parent)
    , m_log_path(paths::writableConfigDir() + QStringLiteral("/lastrun.log"))
    , m_enable_menu_reboot(args.enable_menu_reboot)
    , m_enable_menu_shutdown(args.enable_menu_shutdown)
    , m_enable_menu_suspend(args.enable_menu_suspend)
    , m_enable_menu_appclose(args.enable_menu_appclose)
    , m_enable_menu_settings(args.enable_menu_settings)
{}

void Meta::clearQMLCache()
{
    Log::info(LOGMSG("Reloading the frontend..."));
    emit qmlClearCacheRequested();
}

} // namespace model
