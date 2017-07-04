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
#include "ScriptRunner.h"

#include <QDebug>
#include <QCoreApplication>
#include <QProcess>


namespace ApiParts {

System::System(QObject* parent)
    : QObject(parent)
{
}

void System::quit() const
{
    ScriptRunner::findAndRunSripts(ScriptRunner::EventType::QUIT);

    qInfo().noquote() << tr("Closing Pegasus, goodbye!");
    QCoreApplication::quit();
}

// NOTE: on Linux distros using systemd-logind, local users can
// reboot and shutdown the system without root privileges. Also
// see https://wiki.archlinux.org/index.php/Allow_users_to_shutdown

void System::reboot() const
{
    ScriptRunner::findAndRunSripts({
        ScriptRunner::EventType::QUIT,
        ScriptRunner::EventType::REBOOT
    });


    qInfo().noquote() << tr("Rebooting...");
#ifdef Q_OS_WIN32
    QProcess::startDetached("shutdown -r -t 0");
#else
    QProcess::startDetached("reboot");
#endif
}

void System::shutdown() const
{
    ScriptRunner::findAndRunSripts({
        ScriptRunner::EventType::QUIT,
        ScriptRunner::EventType::SHUTDOWN
    });


    qInfo().noquote() << tr("Shutting down...");
#ifdef Q_OS_WIN32
    QProcess::startDetached("shutdown -s -t 0");
#else
    QProcess::startDetached("poweroff");
#endif
}

} // namespace ApiParts
