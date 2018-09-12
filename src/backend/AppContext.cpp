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


#include "AppContext.h"

#include "Api.h"
#include "AppSettings.h"
#include "LocaleUtils.h"
#include "Paths.h"
#include "ScriptRunner.h"
#include "utils/FolderListModel.h"

#include <QDebug>
#include <QDir>
#include <QQmlEngine>
#include <list>


namespace {

QString find_writable_log_path()
{
    const QString log_path = paths::writableConfigDir();
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
    const QString error_msg = tr_log("Sorry, you cannot create this type in QML.");

    qmlRegisterUncreatableType<model::Collection>(API_URI, 0, 7, "Collection", error_msg);
    qmlRegisterUncreatableType<model::CollectionList>(API_URI, 0, 7, "CollectionList", error_msg);
    qmlRegisterUncreatableType<model::Game>(API_URI, 0, 2, "Game", error_msg);
    qmlRegisterUncreatableType<model::GameList>(API_URI, 0, 6, "GameList", error_msg);
    qmlRegisterUncreatableType<model::GameAssets>(API_URI, 0, 2, "GameAssets", error_msg);
    qmlRegisterUncreatableType<model::Locale>(API_URI, 0, 6, "Locale", error_msg);
    qmlRegisterUncreatableType<model::LocaleList>(API_URI, 0, 6, "LocaleList", error_msg);
    qmlRegisterUncreatableType<model::Theme>(API_URI, 0, 6, "Theme", error_msg);
    qmlRegisterUncreatableType<model::ThemeList>(API_URI, 0, 6, "ThemeList", error_msg);
    qmlRegisterUncreatableType<model::Provider>(API_URI, 0, 9, "Provider", error_msg);
    qmlRegisterUncreatableType<model::ProviderList>(API_URI, 0, 9, "ProviderList", error_msg);
    qmlRegisterUncreatableType<model::Filter>(API_URI, 0, 8, "Filter", error_msg);
    qmlRegisterUncreatableType<model::Filters>(API_URI, 0, 8, "Filters", error_msg);

    // backend classes
    qRegisterMetaType<QVector<model::Collection*>>();
    qRegisterMetaType<QVector<model::Game*>>();

    // QML utilities
    qmlRegisterType<FolderListModel>("Pegasus.FolderListModel", 1, 0, "FolderListModel");
}

} // namespace


namespace backend {

AppContext::AppContext()
{
    setup_logging();
    setup_gamepad();
    register_api_classes();
}

AppContext::~AppContext()
{
    g_log_streams.clear();
}

void AppContext::setup_logging()
{
    if (!AppSettings::silent)
        g_log_streams.emplace_back(stdout);

    qInstallMessageHandler(on_log_message);

    const QString logfile_path = find_writable_log_path();
    if (logfile_path.isEmpty())
        return;

    logfile.setFileName(logfile_path);
    if (!logfile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << tr_log("Could not open `%1` for writing, file logging disabled.")
                      .arg(logfile.fileName());
        return;
    }

    g_log_streams.emplace_back(&logfile);
}

void AppContext::setup_gamepad()
{
    padkeynav.setButtonAKey(Qt::Key_Return);
    padkeynav.setButtonBKey(Qt::Key_Escape);
    padkeynav.setButtonXKey(Qt::Key_I);
    padkeynav.setButtonYKey(Qt::Key_F);
    padkeynav.setButtonL1Key(Qt::Key_A);
    padkeynav.setButtonR1Key(Qt::Key_E);
    padkeynav.setButtonL2Key(Qt::Key_PageUp);
    padkeynav.setButtonR2Key(Qt::Key_PageDown);

    QObject::connect(QGamepadManager::instance(), &QGamepadManager::gamepadAxisEvent,
                     &padaxisnav, &GamepadAxisNavigation::onAxisEvent);

    // config change
    QObject::connect(QGamepadManager::instance(), &QGamepadManager::axisConfigured, on_gamepad_config);
    QObject::connect(QGamepadManager::instance(), &QGamepadManager::buttonConfigured, on_gamepad_config);
}

} // namespace backend
