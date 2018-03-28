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


#include <QDebug>
#include <QProcess>

#ifdef QT_DBUS_LIB
#include <QDBusInterface>
#endif

namespace {

#ifdef QT_DBUS_LIB
bool dbus_call(const char* const service, const char* const path, const char* const interface,
               const QString& call_method, const QVariant& call_arg = QVariant())
{
    QDBusInterface remote(
        QLatin1String(service),
        QLatin1String(path),
        QLatin1String(interface),
        QDBusConnection::systemBus());
    if (!remote.isValid())
        return false;

    remote.call(call_method, call_arg);
    if (remote.lastError().isValid())
        qWarning().noquote() << "[power] Failed to call `" << service << "`: " << remote.lastError().message();

    return !remote.lastError().isValid();
}

// Reboot/shutdown via logind
constexpr auto LOGIND_SERVICE = "org.freedesktop.login1";
constexpr auto LOGIND_PATH = "/org/freedesktop/login1";
constexpr auto LOGIND_IFACE = "org.freedesktop.login1.Manager";
bool shutdown_by_logind()
{
    return dbus_call(LOGIND_SERVICE, LOGIND_PATH, LOGIND_IFACE, "PowerOff", false);
}
bool reboot_by_logind()
{
    return dbus_call(LOGIND_SERVICE, LOGIND_PATH, LOGIND_IFACE, "Reboot", false);
}

// Reboot/shutdown via ConsoleKit
constexpr auto CONSOLEKIT_SERVICE = "org.freedesktop.ConsoleKit";
constexpr auto CONSOLEKIT_PATH = "/org/freedesktop/ConsoleKit/Manager";
constexpr auto CONSOLEKIT_IFACE = "org.freedesktop.ConsoleKit.Manager";
bool shutdown_by_consolekit()
{
    return dbus_call(CONSOLEKIT_SERVICE, CONSOLEKIT_PATH, CONSOLEKIT_IFACE, "Stop");
}
bool reboot_by_consolekit()
{
    return dbus_call(CONSOLEKIT_SERVICE, CONSOLEKIT_PATH, CONSOLEKIT_IFACE, "Restart");
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
