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


#include "Api.h"
#include "FrontendLayer.h"
#include "Model.h"
#include "ProcessLauncher.h"
#include "QuitStatus.h"
#include "ScriptRunner.h"
#include "SystemCommands.h"

#include <QCommandLineParser>
#include <QFile>
#include <QGamepadManager>
#include <QGuiApplication>
#include <QQmlContext>
#include <QSettings>
#include <QStandardPaths>


void handleLogMsg(QtMsgType, const QMessageLogContext&, const QString&);
void handleCommandLineArgs(QGuiApplication&);
void registerAPIClasses();
void setupAsyncGameLaunch(ApiObject&, FrontendLayer&, ProcessLauncher&);
void setupControlsChangeScripts();
void setupQuitScripts(QGuiApplication&);


int main(int argc, char *argv[])
{
    qInstallMessageHandler(handleLogMsg);

    QCoreApplication::addLibraryPath("lib/plugins");
    QCoreApplication::addLibraryPath("lib");

    QGuiApplication app(argc, argv);
    app.setApplicationName("pegasus-frontend");
    app.setApplicationVersion(GIT_REVISION);
    app.setOrganizationName("pegasus-frontend");
    app.setOrganizationDomain("pegasus-frontend.org");

    QSettings::setDefaultFormat(QSettings::IniFormat);

    handleCommandLineArgs(app);


    // this should come before the ApiObject constructor,
    // as it may produce language change signals
    registerAPIClasses();

    // the main parts of the backend
    // frontend <-> api <-> launcher
    ApiObject api;
    FrontendLayer frontend(&api);
    ProcessLauncher launcher;

    setupAsyncGameLaunch(api, frontend, launcher);


    setupControlsChangeScripts();
    setupQuitScripts(app);

    return app.exec();
}

void handleLogMsg(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    // open the output channels: stdout and a file log

    static QTextStream stream_stdout(stdout);

    static QFile logfile([](){
        Q_ASSERT(QStandardPaths::standardLocations(QStandardPaths::AppConfigLocation).length() > 0);

        auto path = QStandardPaths::standardLocations(QStandardPaths::AppConfigLocation).first();
        path += "/lastrun.log";
        path.replace("/pegasus-frontend/pegasus-frontend/", "/pegasus-frontend/");

        return path;
    }());
    static QTextStream stream_logfile([](){
        // the logfile must be available and opened only once;
        // to avoid creating a global or yet another init function,
        // it is handled in this lambda, which will only run once
        Q_ASSERT(!logfile.isOpen());
        logfile.resize(0); // clear previous contents
        logfile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);

        return &logfile;
    }());


    // handle the message
    const QString formattedMsg = qFormatLogMessage(type, context, msg);
    const QByteArray localMsg = formattedMsg.toLocal8Bit();
    stream_stdout << localMsg << endl;
    stream_logfile << localMsg << endl;
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

void registerAPIClasses()
{
    static constexpr auto API_URI = "Pegasus.Model";

    const QString error_msg = QObject::tr("Sorry, you cannot create this type in QML.");

    qmlRegisterUncreatableType<Model::Platform>(API_URI, 0, 2, "Platform", error_msg);
    qmlRegisterUncreatableType<Model::Game>(API_URI, 0, 2, "Game", error_msg);
    qmlRegisterUncreatableType<Model::GameAssets>(API_URI, 0, 2, "GameAssets", error_msg);
    qmlRegisterUncreatableType<ApiParts::Language>(API_URI, 0, 3, "Language", error_msg);
}

void setupAsyncGameLaunch(ApiObject& api, FrontendLayer& frontend, ProcessLauncher& launcher)
{
    // the following communication is required because process handling
    // and destroying/rebuilding the frontend stack are asynchronous tasks;
    // see the relevant classes

    QObject::connect(&api, &ApiObject::prepareLaunch,
                     &frontend, &FrontendLayer::teardown);

    QObject::connect(&frontend, &FrontendLayer::teardownComplete,
                     &api, &ApiObject::onReadyToLaunch);

    QObject::connect(&api, &ApiObject::executeLaunch,
                     &launcher, &ProcessLauncher::launchGame);

    QObject::connect(&launcher, &ProcessLauncher::processFinished,
                     &api, &ApiObject::onGameFinished);

    QObject::connect(&api, &ApiObject::restoreAfterGame,
                     &frontend, &FrontendLayer::rebuild);
}

void setupControlsChangeScripts()
{
    const auto callback = [](){
        using ScriptEvent = ScriptRunner::EventType;

        ScriptRunner::findAndRunScripts(ScriptEvent::CONFIG_CHANGED);
        ScriptRunner::findAndRunScripts(ScriptEvent::CONTROLS_CHANGED);
    };


    QObject::connect(QGamepadManager::instance(), &QGamepadManager::axisConfigured,
                     callback);
    QObject::connect(QGamepadManager::instance(), &QGamepadManager::buttonConfigured,
                     callback);
}

void setupQuitScripts(QGuiApplication& app)
{
    const auto callback = [](){
        using ScriptEvent = ScriptRunner::EventType;

        ScriptRunner::findAndRunScripts(ScriptEvent::QUIT);
        switch (QuitStatus::status) {
            case QuitStatus::Type::REBOOT:
                ScriptRunner::findAndRunScripts(ScriptEvent::REBOOT);
                break;
            case QuitStatus::Type::SHUTDOWN:
                ScriptRunner::findAndRunScripts(ScriptEvent::SHUTDOWN);
                break;
            default:
                break;
        }

        qInfo().noquote() << QObject::tr("Closing Pegasus, goodbye!");

        switch (QuitStatus::status) {
            case QuitStatus::Type::REBOOT:
                SystemCommands::reboot();
                break;
            case QuitStatus::Type::SHUTDOWN:
                SystemCommands::shutdown();
                break;
            default:
                break;
        }
    };


    // run the quit/reboot/shutdown scripts on exit;
    // on some platforms, app.exec() may not return so aboutToQuit()
    // is used for calling these methods
    QObject::connect(&app, &QCoreApplication::aboutToQuit,
                     callback);
}
