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


#pragma once

#include "AppCloseType.h"
#include "api_parts/ApiFilters.h"
#include "api_parts/ApiMeta.h"
#include "api_parts/ApiSettings.h"
#include "model/System.h"
#include "model/PlatformList.h"

#include <QObject>
#include <QQmlListProperty>


/// Provides data access for QML
///
/// Provides an API for the frontend layer, to allow accessing every public
/// property of the backend from QML.
class ApiObject : public QObject {
    Q_OBJECT

    // subcomponents

    Q_PROPERTY(ApiParts::Filters* filters READ filters CONSTANT)
    Q_PROPERTY(ApiParts::Meta* meta READ meta CONSTANT)
    Q_PROPERTY(ApiParts::Settings* settings READ settings CONSTANT)
    Q_PROPERTY(Api::System* system READ system CONSTANT)
    Q_PROPERTY(Model::PlatformList* platforms READ platformList CONSTANT)

    // shortcuts

    Q_PROPERTY(Model::Platform* currentPlatform
               READ currentPlatform
               NOTIFY currentPlatformChanged)
    Q_PROPERTY(Model::Game* currentGame
               READ currentGame
               NOTIFY currentGameChanged)

    // retranslate on locale change
    Q_PROPERTY(QString tr READ emptyString NOTIFY localeChanged)

public:
    explicit ApiObject(QObject* parent = nullptr);

    // game launching
    Q_INVOKABLE void launchGame();

    // scanning
    void startScanning();

    // subcomponents
    ApiParts::Filters* filters() { return &m_filters; }
    ApiParts::Meta* meta() { return &m_meta; }
    ApiParts::Settings* settings() { return &m_settings; }
    Api::System* system() { return &m_system; }
    Model::PlatformList* platformList() { return &m_platform_list; }

    // shortcuts

    Model::Platform* currentPlatform() const { return m_platform_list.current(); }
    Model::Game* currentGame() const {
        return currentPlatform() ? currentPlatform()->gameList().current() : nullptr;
    }

signals:
    // game launching
    void prepareLaunch();
    void executeLaunch(const Model::Platform*, const Model::Game*);
    void restoreAfterGame(ApiObject*);

    // triggers translation update
    void localeChanged();

    // quit/reboot/shutdown
    void appCloseRequested(AppCloseType);

    // shortcuts
    void currentPlatformChanged();
    void currentGameChanged();

public slots:
    // game launch communication
    void onReadyToLaunch();
    void onGameFinished();

private slots:
    // internal communication
    void onScanCompleted();
    void onFiltersChanged();

private:
    ApiParts::Meta m_meta;
    Api::System m_system;
    ApiParts::Settings m_settings;
    ApiParts::Filters m_filters;
    Model::PlatformList m_platform_list;

    // used to trigger re-rendering of texts on locale change
    QString emptyString() const { return QString(); }

};
