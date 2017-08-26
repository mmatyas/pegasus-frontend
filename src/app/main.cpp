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
#include <QSettings>


void handleLogMsg(QtMsgType, const QMessageLogContext&, const QString&);
void handleCommandLineArgs(QGuiApplication&);

// using std::list because QTextStream is not copyable,
// and neither Qt not std::vector can be used in this case
std::list<QTextStream> log_streams;

int main(int argc, char *argv[])
{
    QCoreApplication::addLibraryPath("lib/plugins");
    QCoreApplication::addLibraryPath("lib");

    QGuiApplication app(argc, argv);
    app.setApplicationName("pegasus-frontend");
    app.setApplicationVersion(GIT_REVISION);
    app.setOrganizationName("pegasus-frontend");
    app.setOrganizationDomain("pegasus-frontend.org");
    QSettings::setDefaultFormat(QSettings::IniFormat);

    setupLogStreams(log_streams);
    qInstallMessageHandler(handleLogMsg);

    handleCommandLineArgs(app);


    // this should come before the ApiObject constructor,
    // as it may produce language change signals
    registerAPIClasses();

    // the main parts of the backend
    // frontend <-> api <-> launcher
    ApiObject api;
    FrontendLayer frontend;
    ProcessLauncher launcher;
    setupAsyncGameLaunch(api, frontend, launcher);

    api.startScanning();
    frontend.rebuild(&api);

    setupControlsChangeScripts();
    setupAppCloseScripts(api);

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

void handleLogMsg(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    // forward the message to all registered output streams
    const QByteArray preparedMsg = qFormatLogMessage(type, context, msg).toLocal8Bit();
    for (auto& stream : log_streams)
        stream << preparedMsg << endl;
}
