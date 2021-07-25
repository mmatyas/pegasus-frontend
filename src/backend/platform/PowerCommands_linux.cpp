// Pegasus Frontend
// Copyright (C) 2018  Mátyás Mustoha
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


#include "PowerCommands.h"

#include "Log.h"

#include <QProcess>


namespace {

bool dbus_call(const char* const service, const char* const path, const char* const message,
               const char* const message_arg = nullptr)
{
    const QLatin1String service_str(service);
    const QLatin1String program("dbus-send");
    QStringList args {
        QLatin1String("--system"),
        QLatin1String("--print-reply"),
        QLatin1String("--dest=") + service_str,
        QLatin1String(path),
        QLatin1String(message),
    };
    if (message_arg)
        args << QLatin1String(message_arg);

    QProcess process;
    process.start(program, args, QProcess::ReadOnly);

    const bool success = process.waitForFinished(5000);
    if (!success) {
        Log::warning(LOGMSG("Requesting `%1` from D-Bus service failed.").arg(message));
    }

    return success;
}

// Reboot/shutdown via logind
constexpr auto LOGIND_SERVICE = "org.freedesktop.login1";
constexpr auto LOGIND_PATH = "/org/freedesktop/login1";
constexpr auto LOGIND_FALSE = "boolean:false";
bool shutdown_by_logind()
{
    constexpr auto MESSAGE = "org.freedesktop.login1.Manager.PowerOff";
    return dbus_call(LOGIND_SERVICE, LOGIND_PATH, MESSAGE, LOGIND_FALSE);
}
bool suspend_by_logind()
{
    constexpr auto MESSAGE = "org.freedesktop.login1.Manager.Suspend";
    return dbus_call(LOGIND_SERVICE, LOGIND_PATH, MESSAGE, LOGIND_FALSE);
}
bool reboot_by_logind()
{
    constexpr auto MESSAGE = "org.freedesktop.login1.Manager.Reboot";
    return dbus_call(LOGIND_SERVICE, LOGIND_PATH, MESSAGE, LOGIND_FALSE);
}

// Reboot/shutdown via ConsoleKit
constexpr auto CONSOLEKIT_SERVICE = "org.freedesktop.ConsoleKit";
constexpr auto CONSOLEKIT_PATH = "/org/freedesktop/ConsoleKit/Manager";
bool shutdown_by_consolekit()
{
    constexpr auto MESSAGE = "org.freedesktop.ConsoleKit.Manager.Stop";
    return dbus_call(CONSOLEKIT_SERVICE, CONSOLEKIT_PATH, MESSAGE);
}
bool suspend_by_consolekit()
{
    constexpr auto MESSAGE = "org.freedesktop.ConsoleKit.Manager.Suspend";
    return dbus_call(CONSOLEKIT_SERVICE, CONSOLEKIT_PATH, MESSAGE);
}
bool reboot_by_consolekit()
{
    constexpr auto MESSAGE = "org.freedesktop.ConsoleKit.Manager.Restart";
    return dbus_call(CONSOLEKIT_SERVICE, CONSOLEKIT_PATH, MESSAGE);
}

} // namespace


namespace platform {
namespace power {

void reboot()
{
    if (reboot_by_logind())
        return;
    if (reboot_by_consolekit())
        return;
    QProcess::startDetached(QLatin1String("reboot"), QStringList());
}

void shutdown()
{
    if (shutdown_by_logind())
        return;
    if (shutdown_by_consolekit())
        return;
    QProcess::startDetached(QLatin1String("poweroff"), QStringList());
}

void suspend()
{
    if (suspend_by_logind())
        return;
    if (suspend_by_consolekit())
        return;
    QProcess::startDetached(QLatin1String("suspend"), QStringList());
}
} // namespace power
} // namespace platform
