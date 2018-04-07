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


#include "setup.h"

#include "Api.h"
#include "FrontendLayer.h"
#include "ProcessLauncher.h"

#include <QCommandLineParser>
#include <QGuiApplication>
#include <QIcon>
#include <QSettings>


void handleCommandLineArgs(QGuiApplication&);

int main(int argc, char *argv[])
{
    QCoreApplication::addLibraryPath(QStringLiteral("lib/plugins"));
    QCoreApplication::addLibraryPath(QStringLiteral("lib"));

    QGuiApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("pegasus-frontend"));
    app.setApplicationVersion(QStringLiteral(GIT_REVISION));
    app.setOrganizationName(QStringLiteral("pegasus-frontend"));
    app.setOrganizationDomain(QStringLiteral("pegasus-frontend.org"));
    app.setWindowIcon(QIcon(QStringLiteral(":/icon.png")));
    QSettings::setDefaultFormat(QSettings::IniFormat);

    setupLogStreams();

    handleCommandLineArgs(app);
    setupGamepad();
    // this should come before the ApiObject constructor,
    // as that may produce language change signals
    registerAPIClasses();

    // the main parts of the backend
    // frontend <-> api <-> launcher
    ApiObject api;
    FrontendLayer frontend;
    ProcessLauncher launcher;
    connectAndStartEngine(api, frontend, launcher);

    return app.exec();
}

void handleCommandLineArgs(QGuiApplication& app)
{
    QCommandLineParser argparser;

    //: try to make this less than 80 characters per line
    argparser.setApplicationDescription("\n" + QObject::tr(
        "A cross platform, customizable graphical frontend for launching emulators\n"
        "and managing your game collection."));

    argparser.addHelpOption();
    argparser.addVersionOption();
    argparser.process(app);
}
