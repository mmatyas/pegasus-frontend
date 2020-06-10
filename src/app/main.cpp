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
#include "backend/LocaleUtils.h"
#include "terminal_kbd/TerminalKbd.h"

#include <QCommandLineParser>
#include <QGuiApplication>
#include <QIcon>
#include <QSettings>


backend::CliArgs handle_cli_args(QGuiApplication&);

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(frontend);
    Q_INIT_RESOURCE(themes);
    Q_INIT_RESOURCE(qmlutils);

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

    backend::CliArgs cli_args = handle_cli_args(app);
    backend::Backend backend(cli_args);
    backend.start();

    return app.exec();
}

backend::CliArgs handle_cli_args(QGuiApplication& app)
{
    QCommandLineParser argparser;
    argparser.setApplicationDescription(tr_log(
        "\nPegasus is a graphical frontend for browsing your game library (especially\n"
        "retro games) and launching them from one place. It's focusing on customization,\n"
        "cross platform support (including embedded devices) and high performance."));

    const QCommandLineOption arg_portable(QStringLiteral("portable"),
        tr_log("Do not read or write config files outside the program's directory"));
    argparser.addOption(arg_portable);

    const QCommandLineOption arg_silent(QStringLiteral("silent"),
        tr_log("Do not print log messages to the terminal"));
    argparser.addOption(arg_silent);

    const QCommandLineOption arg_menu_reboot(QStringLiteral("disable-menu-reboot"),
        tr_log("Hides the system reboot entry in the main menu"));
    argparser.addOption(arg_menu_reboot);

    const QCommandLineOption arg_menu_shutdown(QStringLiteral("disable-menu-shutdown"),
        tr_log("Hides the system shutdown entry in the main menu"));
    argparser.addOption(arg_menu_shutdown);

    const QCommandLineOption arg_menu_appclose(QStringLiteral("disable-menu-appclose"),
        tr_log("Hides the closing Pegasus entry in the main menu"));
    argparser.addOption(arg_menu_appclose);

    const QCommandLineOption arg_menu_kiosk(QStringLiteral("kiosk"),
        tr_log("Alias for '--disable-menu-reboot --disable-menu-shutdown --disable-menu-appclose'"));
    argparser.addOption(arg_menu_kiosk);

    argparser.addHelpOption();
    argparser.addVersionOption();
    argparser.process(app); // may quit!

    backend::CliArgs args;
    args.portable = argparser.isSet(arg_portable);
    args.silent = argparser.isSet(arg_silent);
    args.enable_menu_appclose = !(argparser.isSet(arg_menu_kiosk) || argparser.isSet(arg_menu_appclose));
    args.enable_menu_shutdown = !(argparser.isSet(arg_menu_kiosk) || argparser.isSet(arg_menu_shutdown));
    args.enable_menu_reboot = !(argparser.isSet(arg_menu_kiosk) || argparser.isSet(arg_menu_reboot));
    return args;
}
