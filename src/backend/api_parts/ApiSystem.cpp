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


#include "ApiSystem.h"

#include <QDebug>
#include <QDirIterator>
#include <QProcess>
#include <QStandardPaths>


namespace ApiParts {

System::System(QObject* parent)
    : QObject(parent)
{
}

QVector<QString> System::findScripts(const QString& dirname) const
{
    static const auto filters = QDir::Files | QDir::Readable/* | QDir::Executable*/ | QDir::NoDotAndDotDot;
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

void System::runScripts(const QVector<QString>& paths) const
{
    static const auto SCRIPTSTART_MSG = tr("[%1/%2] %3");
    const int num_field_width = QString::number(paths.length()).length();

    qInfo().noquote() << tr("Running scripts...");
    for (int i = 0; i < paths.length(); i++) {
        qInfo().noquote() << SCRIPTSTART_MSG
                             .arg(i + 1, num_field_width)
                             .arg(paths.length()).arg(paths.at(i));
        QProcess::execute(paths.at(i));
    }
}

// NOTE: on Linux distros using systemd-logind, local users can
// reboot and shutdown the system without root privileges. Also
// see https://wiki.archlinux.org/index.php/Allow_users_to_shutdown

void System::reboot() const
{
    QVector<QString> scripts;
    scripts += findScripts("quit");
    scripts += findScripts("reboot");
    runScripts(scripts);


    qInfo().noquote() << tr("Rebooting...");
#ifdef Q_OS_WIN32
    QProcess::startDetached("shutdown -r -t 0");
#else
    QProcess::startDetached("reboot");
#endif
}

void System::shutdown() const
{
    QVector<QString> scripts;
    scripts += findScripts("quit");
    scripts += findScripts("shutdown");
    runScripts(scripts);


    qInfo().noquote() << tr("Shutting down...");
#ifdef Q_OS_WIN32
    QProcess::startDetached("shutdown -s -t 0");
#else
    QProcess::startDetached("poweroff");
#endif
}

} // namespace ApiParts
