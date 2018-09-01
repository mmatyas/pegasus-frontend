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


#include "AppArgs.h"
#include "AppContext.h"
#include "Backend.h"
#include "LocaleUtils.h"

#include <QCommandLineParser>
#include <QGuiApplication>
#include <QIcon>
#include <QSettings>


void handleCommandLineArgs(QGuiApplication&);

int main(int argc, char *argv[])
{
    QCoreApplication::addLibraryPath(QStringLiteral("lib/plugins"));
    QCoreApplication::addLibraryPath(QStringLiteral("lib"));
    QSettings::setDefaultFormat(QSettings::IniFormat);

    QGuiApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("pegasus-frontend"));
    app.setApplicationVersion(QStringLiteral(GIT_REVISION));
    app.setOrganizationName(QStringLiteral("pegasus-frontend"));
    app.setOrganizationDomain(QStringLiteral("pegasus-frontend.org"));
    app.setWindowIcon(QIcon(QStringLiteral(":/icon.png")));

    AppArgs::load_config();
    handleCommandLineArgs(app);

    backend::AppContext context;
    backend::Backend backend;
    backend.start();

    return app.exec();
}

void handleCommandLineArgs(QGuiApplication& app)
{
    QCommandLineParser argparser;

    argparser.setApplicationDescription('\n' + tr_log(
        "A cross platform, customizable graphical frontend for launching emulators "
        "and managing your game collection."));

    const QCommandLineOption arg_portable(QStringLiteral("portable"),
        tr_log("Do not read or write config files outside the program's directory"));
    argparser.addOption(arg_portable);

    const QCommandLineOption arg_silent(QStringLiteral("silent"),
        tr_log("Do not print log messages to the terminal"));
    argparser.addOption(arg_silent);

    argparser.addHelpOption();
    argparser.addVersionOption();
    argparser.process(app);


    AppArgs::portable_mode = argparser.isSet(arg_portable);
    AppArgs::silent = argparser.isSet(arg_silent);
}
