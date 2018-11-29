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


#include "Backend.h"

#include "LocaleUtils.h"
#include "Log.h"
#include "ScriptRunner.h"
#include "platform/PowerCommands.h"

#include <QCoreApplication>
#include <QDebug>


namespace {

void on_app_close(AppCloseType type)
{
    ScriptRunner::run(ScriptEvent::QUIT);
    switch (type) {
        case AppCloseType::REBOOT:
            ScriptRunner::run(ScriptEvent::REBOOT);
            break;
        case AppCloseType::SHUTDOWN:
            ScriptRunner::run(ScriptEvent::SHUTDOWN);
            break;
        default: break;
    }

    qInfo().noquote() << tr_log("Closing Pegasus, goodbye!");
    Log::close();

    QCoreApplication::quit();
    switch (type) {
        case AppCloseType::REBOOT:
            platform::power::reboot();
            break;
        case AppCloseType::SHUTDOWN:
            platform::power::shutdown();
            break;
        default: break;
    }
}

} // namespace


namespace backend {

Backend::Backend()
    : frontend(&api)
{
    // the following communication is required because process handling
    // and destroying/rebuilding the frontend stack are asynchronous tasks;
    // see the relevant classes

    // the Api asks the Launcher to start the game
    QObject::connect(&api, &ApiObject::launchGame,
                     &launcher, &ProcessLauncher::onLaunchRequested);

    // the Launcher tries to start the game, ask the Frontend
    // to tear down the UI, then report back to the Api
    QObject::connect(&launcher, &ProcessLauncher::processLaunchOk,
                     &api, &ApiObject::onGameLaunchOk);

    QObject::connect(&launcher, &ProcessLauncher::processLaunchError,
                     &api, &ApiObject::onGameLaunchError);

    QObject::connect(&launcher, &ProcessLauncher::processLaunchOk,
                     &frontend, &FrontendLayer::teardown);

    QObject::connect(&frontend, &FrontendLayer::teardownComplete,
                     &launcher, &ProcessLauncher::onTeardownComplete);

    // when the game ends, the Launcher wakes up the Api and the Frontend
    QObject::connect(&launcher, &ProcessLauncher::processFinished,
                     &api, &ApiObject::onGameFinished);

    QObject::connect(&launcher, &ProcessLauncher::processFinished,
                     &frontend, &FrontendLayer::rebuild);


    // partial QML reload
    QObject::connect(api.internal().metaPtr(), &model::Meta::qmlClearCacheRequested,
                     &frontend, &FrontendLayer::clearCache);

    // quit/reboot/shutdown request
    QObject::connect(api.internal().systemPtr(), &model::System::appCloseRequested, on_app_close);
}

void Backend::start()
{
    frontend.rebuild();
    api.startScanning();
}

} // namespace backend
