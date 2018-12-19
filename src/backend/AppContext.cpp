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
#include "Log.h"
#include "Paths.h"
#include "ScriptRunner.h"
#include "model/gaming/Collection.h"
#include "model/gaming/Game.h"
#include "model/gaming/GameAssets.h"
#include "model/keys/Key.h"
#include "utils/FolderListModel.h"

#include "QtQmlTricks/QQmlObjectListModel.h"
#include <QDebug>
#include <QDir>
#include <QGuiApplication>
#include <QQmlEngine>
#include <QSysInfo>
#include <list>


namespace {

void print_metainfo()
{
    Log::info(tr_log("Pegasus " GIT_REVISION " (" GIT_DATE ")"));
    Log::info(tr_log("Running on %1 (%2, %3)").arg(
        QSysInfo::prettyProductName(),
        QSysInfo::currentCpuArchitecture(),
        QGuiApplication::platformName()));
}

void on_gamepad_config()
{
    ScriptRunner::run(ScriptEvent::CONFIG_CHANGED);
    ScriptRunner::run(ScriptEvent::CONTROLS_CHANGED);
}

void register_api_classes()
{
    // register API classes:
    //   this should come before the ApiObject constructor,
    //   as that may produce language change signals

    constexpr auto API_URI = "Pegasus.Model";
    const QString error_msg = tr_log("Sorry, you cannot create this type in QML.");

    qmlRegisterUncreatableType<model::Collection>(API_URI, 0, 7, "Collection", error_msg);
    qmlRegisterUncreatableType<model::Game>(API_URI, 0, 2, "Game", error_msg);
    qmlRegisterUncreatableType<model::GameAssets>(API_URI, 0, 2, "GameAssets", error_msg);
    qmlRegisterUncreatableType<model::Locales>(API_URI, 0, 11, "Locales", error_msg);
    qmlRegisterUncreatableType<model::Themes>(API_URI, 0, 11, "Themes", error_msg);
    qmlRegisterUncreatableType<model::Providers>(API_URI, 0, 11, "Providers", error_msg);
    qmlRegisterUncreatableType<model::Key>(API_URI, 0, 10, "Key", error_msg);
    qmlRegisterUncreatableType<model::Keys>(API_URI, 0, 10, "Keys", error_msg);

    // QML utilities
    qmlRegisterType<FolderListModel>("Pegasus.FolderListModel", 1, 0, "FolderListModel");

    // third-party
    qmlRegisterUncreatableType<QQmlObjectListModelBase>("QtQmlTricks.SmartDataModels",
                                                        2, 0, "ObjectListModel", error_msg);
}

} // namespace


namespace backend {

AppContext::AppContext()
{
    print_metainfo();
    setup_gamepad();
    register_api_classes();
}

void AppContext::setup_gamepad()
{
    #define SET_GAMEPAD_KEY(fnName, enumName) \
        padkeynav.setButton ## fnName ## Key(static_cast<Qt::Key>(GamepadKeyId::enumName));
    SET_GAMEPAD_KEY(A, A);
    SET_GAMEPAD_KEY(B, B);
    SET_GAMEPAD_KEY(X, X);
    SET_GAMEPAD_KEY(Y, Y);
    SET_GAMEPAD_KEY(L1, L1);
    SET_GAMEPAD_KEY(L2, L2);
    SET_GAMEPAD_KEY(L3, L3);
    SET_GAMEPAD_KEY(R1, R1);
    SET_GAMEPAD_KEY(R2, R2);
    SET_GAMEPAD_KEY(R3, R3);
    SET_GAMEPAD_KEY(Select, SELECT);
    SET_GAMEPAD_KEY(Start, START);
    SET_GAMEPAD_KEY(Guide, GUIDE);
    #undef SET_GAMEPAD_KEY

#ifndef Q_OS_ANDROID
    QObject::connect(QGamepadManager::instance(), &QGamepadManager::gamepadAxisEvent,
                     &padaxisnav, &GamepadAxisNavigation::onAxisEvent);
#endif

    // config change
    QObject::connect(QGamepadManager::instance(), &QGamepadManager::axisConfigured, on_gamepad_config);
    QObject::connect(QGamepadManager::instance(), &QGamepadManager::buttonConfigured, on_gamepad_config);
}

} // namespace backend
