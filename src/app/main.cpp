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


#include "backend/Backend.h"
#include "backend/platform/TerminalKbd.h"

#include <QCommandLineParser>
#include <QFileInfo>
#include <QGuiApplication>
#include <QIcon>
#include <QSettings>
#include <QtPlugin>

#ifdef Q_OS_ANDROID
#include "backend/platform/AndroidHelpers.h"
#endif


#ifdef WITH_APNG_SUPPORT
Q_IMPORT_PLUGIN(ApngImagePlugin)
#endif


backend::CliArgs handle_cli_args(QGuiApplication&);
bool request_runtime_permissions();
bool portable_txt_present();

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(frontend);
    Q_INIT_RESOURCE(themes);
    Q_INIT_RESOURCE(qmlutils);
#ifdef PEGASUS_USING_CMAKE  // TODO: Unify the build system
    Q_INIT_RESOURCE(assets);
    Q_INIT_RESOURCE(locales);
    Q_INIT_RESOURCE(qmlutils_qmlcache);
    Q_INIT_RESOURCE(frontend_qmlcache);
    Q_INIT_RESOURCE(themes_qmlcache);
#endif

    TerminalKbd::on_startup();

    QCoreApplication::addLibraryPath(QStringLiteral("lib/plugins"));
    QCoreApplication::addLibraryPath(QStringLiteral("lib"));
    QSettings::setDefaultFormat(QSettings::IniFormat);

    QGuiApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("pegasus-frontend"));
    app.setApplicationVersion(QStringLiteral(GIT_REVISION));
    app.setOrganizationName(QStringLiteral("pegasus-frontend"));
    app.setOrganizationDomain(QStringLiteral("pegasus-frontend.org"));
    app.setWindowIcon(QIcon(QStringLiteral(":/icon.png")));

    if (!request_runtime_permissions())
        return 1;

    backend::CliArgs cli_args = handle_cli_args(app);
    cli_args.portable |= portable_txt_present();

    backend::Backend backend(cli_args);
    backend.start();

    return app.exec();
}

bool request_runtime_permissions()
{
#ifdef Q_OS_ANDROID
    return android::has_external_storage_access();
#endif

    return true;
}


bool portable_txt_present()
{
#if defined(Q_OS_ANDROID) || defined(Q_OS_MACOS)
    // NOTE: On Android, the executable location is not generally accessible,
    // while on Mac it is inside the bundle
    return false;
#else
    const QString path = QCoreApplication::applicationDirPath() + QStringLiteral("/portable.txt");
    return QFileInfo::exists(path);
#endif
}


QCommandLineOption add_cli_option(QCommandLineParser& parser, const QString& name, const QString& desc)
{
    QCommandLineOption arg(name, desc);
    parser.addOption(arg);
    return arg;
}

backend::CliArgs handle_cli_args(QGuiApplication& app)
{
#define CMDMSG QStringLiteral

    QCommandLineParser argparser;
    argparser.setApplicationDescription(CMDMSG(
        "\nPegasus is a graphical frontend for browsing your game library (especially\n"
        "retro games) and launching them from one place. It's focusing on customization,\n"
        "cross platform support (including embedded devices) and high performance."));

    const QCommandLineOption arg_portable = add_cli_option(argparser,
        QStringLiteral("portable"),
        CMDMSG("Do not read or write config files outside the program's directory"));

    const QCommandLineOption arg_silent = add_cli_option(argparser,
        QStringLiteral("silent"),
        CMDMSG("Do not print log messages to the terminal"));

    const QCommandLineOption arg_menu_reboot = add_cli_option(argparser,
        QStringLiteral("disable-menu-reboot"),
        CMDMSG("Hides the system reboot entry in the main menu"));

    const QCommandLineOption arg_menu_shutdown = add_cli_option(argparser,
        QStringLiteral("disable-menu-shutdown"),
        CMDMSG("Hides the system shutdown entry in the main menu"));

    const QCommandLineOption arg_menu_appclose = add_cli_option(argparser,
        QStringLiteral("disable-menu-appclose"),
        CMDMSG("Hides the closing Pegasus entry in the main menu"));

    const QCommandLineOption arg_menu_settings = add_cli_option(argparser,
        QStringLiteral("disable-menu-settings"),
        CMDMSG("Hides the settings menu entry in the main menu"));

    const QCommandLineOption arg_menu_kiosk = add_cli_option(argparser,
        QStringLiteral("kiosk"),
        CMDMSG("Alias for:\n"
               "--disable-menu-reboot\n"
               "--disable-menu-shutdown\n"
               "--disable-menu-appclose\n"
               "--disable-menu-settings"));

    const QCommandLineOption arg_gamepad_autoconfig = add_cli_option(argparser,
        QStringLiteral("disable-gamepad-autoconfig"),
        CMDMSG("Disables the automatic layout detection for connected gamepads.\n"
               "When you connect a gamepad, Pegasus tries to guess its button and axis layout "
               "automatically based on a list of known devices. Unfortunately this doesn't seem "
               "to work perfectly with some platforms and devices (eg. arcades), in which case "
               "you can disable this feature here."));

    argparser.addHelpOption();
    argparser.addVersionOption();
    argparser.process(app); // may quit!

    backend::CliArgs args;
    args.portable = argparser.isSet(arg_portable);
    args.silent = argparser.isSet(arg_silent);
    args.enable_menu_appclose = !(argparser.isSet(arg_menu_kiosk) || argparser.isSet(arg_menu_appclose));
    args.enable_menu_settings = !(argparser.isSet(arg_menu_kiosk) || argparser.isSet(arg_menu_settings));
    args.enable_gamepad_autoconfig = !argparser.isSet(arg_gamepad_autoconfig);
#ifdef Q_OS_ANDROID
    args.enable_menu_shutdown = false;
    args.enable_menu_reboot = false;
#else
    args.enable_menu_shutdown = !(argparser.isSet(arg_menu_kiosk) || argparser.isSet(arg_menu_shutdown));
    args.enable_menu_reboot = !(argparser.isSet(arg_menu_kiosk) || argparser.isSet(arg_menu_reboot));
#endif
    return args;

#undef CMDMSG
}
