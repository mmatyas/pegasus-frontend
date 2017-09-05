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
#include "AppCloseType.h"
#include "FrontendLayer.h"
#include "ProcessLauncher.h"
#include "ScriptRunner.h"
#include "SystemCommands.h"
#include "model/Platform.h"

#include <QDir>
#include <QFile>
#include <QGamepadKeyNavigation>
#include <QGamepadManager>
#include <QGuiApplication>
#include <QQmlContext>
#include <QRegularExpression>
#include <QStandardPaths>


void setupLogStreams(std::list<QTextStream>& streams)
{
    streams.emplace_back(stdout);

    using regex = QRegularExpression;
    using qsp = QStandardPaths;

    Q_ASSERT(qsp::standardLocations(qsp::AppConfigLocation).length() > 0);
    QString log_path = qsp::standardLocations(qsp::AppConfigLocation).first();
    log_path = log_path.replace(regex("/pegasus-frontend/pegasus-frontend$"), "/pegasus-frontend");

    if (!QDir().mkpath(log_path)) {
        streams.back() << QObject::tr("Warning: `%1` does not exists, and could not create it."
                                      " File logging disabled.").arg(log_path) << endl;
        return;
    }

    log_path += "/lastrun.log";

    static QFile log_file(log_path);
    log_file.open(QIODevice::WriteOnly | QIODevice::Text);

    streams.emplace_back(&log_file);
}

void setupGamepadNavigation()
{
    static QGamepadKeyNavigation padkeynav;

    padkeynav.setButtonAKey(Qt::Key_Return);
    padkeynav.setButtonBKey(Qt::Key_Escape);
    padkeynav.setButtonXKey(Qt::Key_Control);
    padkeynav.setButtonL1Key(Qt::Key_A);
    padkeynav.setButtonL2Key(Qt::Key_A);
    padkeynav.setButtonR1Key(Qt::Key_D);
    padkeynav.setButtonR2Key(Qt::Key_D);
}

void registerAPIClasses()
{
    constexpr auto API_URI = "Pegasus.Model";

    const QString error_msg = QObject::tr("Sorry, you cannot create this type in QML.");

    qmlRegisterUncreatableType<Model::Platform>(API_URI, 0, 2, "Platform", error_msg);
    qmlRegisterUncreatableType<Model::Game>(API_URI, 0, 2, "Game", error_msg);
    qmlRegisterUncreatableType<Model::GameAssets>(API_URI, 0, 2, "GameAssets", error_msg);
    qmlRegisterUncreatableType<Model::Locale>(API_URI, 0, 5, "Locale", error_msg);
    qmlRegisterUncreatableType<Model::Theme>(API_URI, 0, 4, "Theme", error_msg);
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

    QObject::connect(QGamepadManager::instance(), &QGamepadManager::axisConfigured, callback);
    QObject::connect(QGamepadManager::instance(), &QGamepadManager::buttonConfigured, callback);
}

void setupAppCloseScripts(ApiObject& api)
{
    const auto callback = [](AppCloseType type){
        using ScriptEvent = ScriptRunner::EventType;

        ScriptRunner::findAndRunScripts(ScriptEvent::QUIT);
        switch (type) {
            case AppCloseType::REBOOT:
                ScriptRunner::findAndRunScripts(ScriptEvent::REBOOT);
                break;
            case AppCloseType::SHUTDOWN:
                ScriptRunner::findAndRunScripts(ScriptEvent::SHUTDOWN);
                break;
            default: break;
        }

        qInfo().noquote() << QObject::tr("Closing Pegasus, goodbye!");

        QCoreApplication::quit();
        switch (type) {
            case AppCloseType::REBOOT:
                SystemCommands::reboot();
                break;
            case AppCloseType::SHUTDOWN:
                SystemCommands::shutdown();
                break;
            default: break;
        }
    };

    // do the connection
    QObject::connect(&api, &ApiObject::appCloseRequested, callback);
}
