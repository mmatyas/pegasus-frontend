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


#include <QProcess>

#ifdef QT_DBUS_LIB
#include <QDBusInterface>
#endif

namespace {

#ifdef QT_DBUS_LIB
constexpr auto LOGIND_SERVICE = "org.freedesktop.login1";
constexpr auto LOGIND_PATH = "/org/freedesktop/login1";
constexpr auto LOGIND_IFACE = "org.freedesktop.login1.Manager";

bool shutdown_by_logind()
{
    QDBusInterface iface(
        QLatin1String(LOGIND_SERVICE),
        QLatin1String(LOGIND_PATH),
        QLatin1String(LOGIND_IFACE),
        QDBusConnection::systemBus());
    if (iface.isValid()) {
        iface.call("PowerOff", false);
        return !iface.lastError().isValid();
    }

    return false;
}
bool reboot_by_logind()
{
    QDBusInterface iface(
        QLatin1String(LOGIND_SERVICE),
        QLatin1String(LOGIND_PATH),
        QLatin1String(LOGIND_IFACE),
        QDBusConnection::systemBus());
    if (iface.isValid()) {
        iface.call("Reboot", false);
        return !iface.lastError().isValid();
    }

    return false;
}

constexpr auto CONSOLEKIT_SERVICE = "org.freedesktop.ConsoleKit";
constexpr auto CONSOLEKIT_PATH = "/org/freedesktop/ConsoleKit/Manager";
constexpr auto CONSOLEKIT_IFACE = "org.freedesktop.ConsoleKit.Manager";

bool shutdown_by_consolekit()
{
    QDBusInterface iface(
        QLatin1String(CONSOLEKIT_SERVICE),
        QLatin1String(CONSOLEKIT_PATH),
        QLatin1String(CONSOLEKIT_IFACE),
        QDBusConnection::systemBus());
    if (iface.isValid()) {
        iface.call("Stop");
        return !iface.lastError().isValid();
    }

    return false;
}
bool reboot_by_consolekit()
{
    QDBusInterface iface(
        QLatin1String(CONSOLEKIT_SERVICE),
        QLatin1String(CONSOLEKIT_PATH),
        QLatin1String(CONSOLEKIT_IFACE),
        QDBusConnection::systemBus());
    if (iface.isValid()) {
        iface.call("Restart");
        return !iface.lastError().isValid();
    }

    return false;
}
#endif // QT_DBUS_LIB

} // namespace


namespace platform {
namespace power {

void reboot()
{
#ifdef QT_DBUS_LIB
    if (reboot_by_logind())
        return;
    if (reboot_by_consolekit())
        return;
#endif
    QProcess::startDetached("reboot");
}

void shutdown()
{
#ifdef QT_DBUS_LIB
    if (shutdown_by_logind())
        return;
    if (shutdown_by_consolekit())
        return;
#endif
    QProcess::startDetached("poweroff");
}

} // namespace power
} // namespace platform
