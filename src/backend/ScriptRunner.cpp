// Pegasus Frontend
// Copyright (C) 2017-2018  Mátyás Mustoha
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


#include "ScriptRunner.h"

#include "LocaleUtils.h"
#include "Paths.h"
#include "utils/HashMap.h"

#include <QDebug>
#include <QDirIterator>
#include <QProcess>
#include <QString>
#include <QStringBuilder>
#include <vector>


namespace {
std::vector<QString> find_scripts_in(const QString& dirname)
{
    constexpr auto filters = QDir::Files | QDir::Readable | QDir::Executable | QDir::NoDotAndDotDot;
    constexpr auto flags = QDirIterator::Subdirectories | QDirIterator::FollowSymlinks;

    Q_ASSERT(!dirname.isEmpty());

    std::vector<QString> all_scripts;

    const QStringList configdirs = paths::configDirs();
    for (const QString& configdir : configdirs) {
        const QString scriptdir = configdir % QStringLiteral("/scripts/") % dirname;

        std::vector<QString> local_scripts;
        QDirIterator scripdir_it(scriptdir, filters, flags);
        while (scripdir_it.hasNext())
            local_scripts.emplace_back(scripdir_it.next());

        std::sort(local_scripts.begin(), local_scripts.end());
        all_scripts.insert(all_scripts.end(),
                           std::make_move_iterator(local_scripts.begin()),
                           std::make_move_iterator(local_scripts.end()));
    }

    return all_scripts;
}

void execute_all(const std::vector<QString>& paths)
{
    Q_ASSERT(!paths.empty());

    const int num_field_width = QString::number(paths.size()).length();

    for (size_t i = 0; i < paths.size(); i++) {
        qInfo().noquote() << tr_log("[%1/%2] Running `%3`")
            .arg(i + 1, num_field_width)
            .arg(paths.size())
            .arg(paths[i]);
        QProcess::execute(paths[i]);
    }
}
} // namespace


void ScriptRunner::run(ScriptEvent event)
{
    static const HashMap<ScriptEvent, QString, EnumHash> SCRIPT_DIRS = {
        { ScriptEvent::QUIT, QStringLiteral("quit") },
        { ScriptEvent::REBOOT, QStringLiteral("reboot") },
        { ScriptEvent::SHUTDOWN, QStringLiteral("shutdown") },
        { ScriptEvent::CONFIG_CHANGED, QStringLiteral("config-changed") },
        { ScriptEvent::SETTINGS_CHANGED, QStringLiteral("settings-changed") },
        { ScriptEvent::CONTROLS_CHANGED, QStringLiteral("controls-changed") },
        { ScriptEvent::PROCESS_STARTED, QStringLiteral("game-start") },
        { ScriptEvent::PROCESS_FINISHED, QStringLiteral("game-end") },
    };
    Q_ASSERT(SCRIPT_DIRS.count(event));


    const QString& dirname = SCRIPT_DIRS.at(event);
    const std::vector<QString> scripts = find_scripts_in(dirname);

    if (scripts.empty())
        return;

    qInfo().noquote() << tr_log("Running `%1` scripts...").arg(dirname);
    execute_all(scripts);
}
