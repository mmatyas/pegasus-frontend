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

#include "Api.h"
#include "ScriptRunner.h"
#include "platform/PowerCommands.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QQmlEngine>
#include <QRegularExpression>
#include <QStandardPaths>


namespace {

QString find_writable_config_dir()
{
    const QRegularExpression replace_regex(QStringLiteral("/pegasus-frontend/pegasus-frontend$"));
    const QString dir_path = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)
        .replace(replace_regex, QStringLiteral("/pegasus-frontend"));
    if (dir_path.isEmpty()) {
        qWarning() << QObject::tr("No writable location found to save logs, file logging disabled.");
        return QString();
    }

    QDir dir(dir_path);
    if (!dir.mkpath(QLatin1String("."))) { // also true if already exists
        qWarning() << QObject::tr("Could create `%1`, file logging disabled.").arg(dir_path);
        return QString();
    }

    return dir_path;
}

QString find_writable_log_path()
{
    const QString log_path = find_writable_config_dir();
    if (log_path.isEmpty())
        return QString();

    return log_path + QLatin1String("/lastrun.log");
}

// using std::list because QTextStream is not copyable or movable,
// and neither Qt not std::vector can be used in this case
std::list<QTextStream> g_log_streams;

void on_log_message(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    // forward the message to all registered output streams
    const QByteArray preparedMsg = qFormatLogMessage(type, context, msg).toLocal8Bit();
    for (auto& stream : g_log_streams)
        stream << preparedMsg << endl;
}


void on_gamepad_config()
{
    using ScriptEvent = ScriptRunner::EventType;

    ScriptRunner::findAndRunScripts(ScriptEvent::CONFIG_CHANGED);
    ScriptRunner::findAndRunScripts(ScriptEvent::CONTROLS_CHANGED);
}

void register_api_classes()
{
    // register API classes:
    //   this should come before the ApiObject constructor,
    //   as that may produce language change signals

    constexpr auto API_URI = "Pegasus.Model";
    const QString error_msg = QObject::tr("Sorry, you cannot create this type in QML.");

    qmlRegisterUncreatableType<Types::Collection>(API_URI, 0, 7, "Collection", error_msg);
    qmlRegisterUncreatableType<Types::CollectionList>(API_URI, 0, 7, "CollectionList", error_msg);
    qmlRegisterUncreatableType<Types::Game>(API_URI, 0, 2, "Game", error_msg);
    qmlRegisterUncreatableType<Types::GameList>(API_URI, 0, 6, "GameList", error_msg);
    qmlRegisterUncreatableType<Types::GameAssets>(API_URI, 0, 2, "GameAssets", error_msg);
    qmlRegisterUncreatableType<Types::Locale>(API_URI, 0, 6, "Locale", error_msg);
    qmlRegisterUncreatableType<Types::LocaleList>(API_URI, 0, 6, "LocaleList", error_msg);
    qmlRegisterUncreatableType<Types::Theme>(API_URI, 0, 6, "Theme", error_msg);
    qmlRegisterUncreatableType<Types::ThemeList>(API_URI, 0, 6, "ThemeList", error_msg);
}

void on_app_close(AppCloseType type)
{
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

Context::Context()
{
    setup_logging();
    setup_gamepad();
    register_api_classes();
}

Context::~Context()
{
    g_log_streams.clear();
}

void Context::setup_logging()
{
    g_log_streams.emplace_back(stdout);
    qInstallMessageHandler(on_log_message);

    const QString logfile_path = find_writable_log_path();
    if (logfile_path.isEmpty())
        return;

    logfile.setFileName(logfile_path);
    if (!logfile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << QObject::tr("Could not open `%1` for writing, file logging disabled.")
                      .arg(logfile.fileName());
        return;
    }

    g_log_streams.emplace_back(&logfile);
}

void Context::setup_gamepad()
{
    padkeynav.setButtonAKey(Qt::Key_Return);
    padkeynav.setButtonBKey(Qt::Key_Escape);
    padkeynav.setButtonXKey(Qt::Key_Control);
    padkeynav.setButtonL1Key(Qt::Key_A);
    padkeynav.setButtonR1Key(Qt::Key_D);
    padkeynav.setButtonL2Key(Qt::Key_PageUp);
    padkeynav.setButtonR2Key(Qt::Key_PageDown);

    QObject::connect(QGamepadManager::instance(), &QGamepadManager::gamepadAxisEvent,
                     &padaxisnav, &GamepadAxisNavigation::onAxisEvent);

    // config change
    QObject::connect(QGamepadManager::instance(), &QGamepadManager::axisConfigured, on_gamepad_config);
    QObject::connect(QGamepadManager::instance(), &QGamepadManager::buttonConfigured, on_gamepad_config);
}

Backend::Backend()
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

    // special commands
    QObject::connect(&api, &ApiObject::qmlClearCacheRequested,
                     &frontend, &FrontendLayer::clearCache);

    // close the app on quit request
    QObject::connect(&api, &ApiObject::appCloseRequested, on_app_close);
}

void Backend::start()
{
    frontend.rebuild(&api);
    api.startScanning();
}

} // namespace backend
