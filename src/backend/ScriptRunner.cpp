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


#include "ScriptRunner.h"

#include <QDebug>
#include <QDirIterator>
#include <QProcess>
#include <QStandardPaths>


const QMap<ScriptRunner::EventType, QString> ScriptRunner::script_dirs = {
    { ScriptRunner::EventType::QUIT, "quit" },
    { ScriptRunner::EventType::REBOOT, "reboot" },
    { ScriptRunner::EventType::SHUTDOWN, "shutdown" },
    { ScriptRunner::EventType::CONFIG_CHANGED, "config-changed" },
    { ScriptRunner::EventType::SETTINGS_CHANGED, "settings-changed" },
    { ScriptRunner::EventType::CONTROLS_CHANGED, "controls-changed" },
};

void ScriptRunner::findAndRunScripts(ScriptRunner::EventType event)
{
    Q_ASSERT(script_dirs.contains(event));

    const auto scripts = findScripts(event);
    const auto dirname = script_dirs.value(event);

    if (scripts.count() > 0) {
        qInfo().noquote() << QObject::tr("Running `%1` scripts...").arg(dirname);
        runScripts(scripts);
    }
}

QVector<QString> ScriptRunner::findScripts(ScriptRunner::EventType event)
{
    Q_ASSERT(script_dirs.contains(event));

    return findScripts(script_dirs.value(event));
}

QVector<QString> ScriptRunner::findScripts(const QString& dirname)
{
    static const auto filters = QDir::Files | QDir::Readable | QDir::Executable | QDir::NoDotAndDotDot;
    static const auto flags = QDirIterator::Subdirectories | QDirIterator::FollowSymlinks;

    Q_ASSERT(!dirname.isEmpty());

    QVector<QString> scripts;

    auto search_paths = QStandardPaths::standardLocations(QStandardPaths::AppConfigLocation);
    for (auto& path : search_paths) {
        path += "/scripts/" + dirname;
        // do not add the organization name to the search path
        path.replace("/pegasus-frontend/pegasus-frontend/", "/pegasus-frontend/");

        QVector<QString> local_scripts;
        QDirIterator scripdir(path, filters, flags);
        while (scripdir.hasNext())
            local_scripts.append(scripdir.next());

        std::sort(local_scripts.begin(), local_scripts.end());
        scripts.append(local_scripts);
    }

    return scripts;
}

void ScriptRunner::runScripts(const QVector<QString>& paths)
{
    if (paths.length() <= 0)
        return;

    static const auto SCRIPTSTART_MSG = QObject::tr("[%1/%2] %3");
    const int num_field_width = QString::number(paths.length()).length();

    for (int i = 0; i < paths.length(); i++) {
        qInfo().noquote() << SCRIPTSTART_MSG
                             .arg(i + 1, num_field_width)
                             .arg(paths.length()).arg(paths.at(i));
        QProcess::execute(paths.at(i));
    }
}
