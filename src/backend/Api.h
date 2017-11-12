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
#include "api_parts/ApiPlatforms.h"
#include "api_parts/ApiSettings.h"
#include "api_parts/ApiSystem.h"
#include "model/Platform.h"

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
    Q_PROPERTY(ApiParts::System* system READ system CONSTANT)

    // shortcuts
    // it feels more natural to use api.platforms and api.platforms[i].games
    // as a model than api.platforms.all

    Q_PROPERTY(QQmlListProperty<Model::Platform> platforms
               READ getPlatformsProp
               NOTIFY platformModelChanged)
    Q_PROPERTY(Model::Platform* currentPlatform
               READ currentPlatform
               NOTIFY currentPlatformChanged)
    Q_PROPERTY(int currentPlatformIndex
               READ currentPlatformIndex
               WRITE setCurrentPlatformIndex
               NOTIFY currentPlatformIndexChanged)
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
    ApiParts::Platforms* platformTable() { return &m_platforms; }
    ApiParts::Settings* settings() { return &m_settings; }
    ApiParts::System* system() { return &m_system; }

    // shortcuts

    QQmlListProperty<Model::Platform> getPlatformsProp() { return m_platforms.getListProp(); }
    int currentPlatformIndex() const { return m_platforms.currentIndex(); }
    void setCurrentPlatformIndex(int idx) { m_platforms.setCurrentIndex(idx); }

    Model::Platform* currentPlatform() const { return m_platforms.currentPlatform(); }
    Model::Game* currentGame() const {
        return currentPlatform() ? currentPlatform()->currentGame() : nullptr;
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
    void platformModelChanged();
    void currentPlatformIndexChanged();
    void currentPlatformChanged();
    void currentGameChanged();

public slots:
    // game launch communication
    void onReadyToLaunch();
    void onGameFinished();

private slots:
    // internal communication
    void onScanCompleted();

private:
    ApiParts::Meta m_meta;
    ApiParts::Platforms m_platforms;
    ApiParts::System m_system;
    ApiParts::Settings m_settings;
    ApiParts::Filters m_filters;

    // used to trigger re-rendering of texts on locale change
    QString emptyString() const { return QString(); }
};
