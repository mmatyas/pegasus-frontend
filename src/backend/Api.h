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

#include "Model.h"
#include "api_parts/ApiFilters.h"
#include "api_parts/ApiMeta.h"
#include "api_parts/ApiPlatforms.h"
#include "api_parts/ApiSettings.h"
#include "api_parts/ApiSystem.h"

#include <QObject>
#include <QQmlListProperty>


/// Provides data access for QML
///
/// Provides an API for the frontend layer, to allow accessing every public
/// property of the backend from QML.
class ApiObject : public QObject {
    Q_OBJECT

    // platform-related properties
    Q_PROPERTY(QQmlListProperty<Model::Platform> platforms
               READ getPlatformsProp
               NOTIFY platformModelChanged)
    Q_PROPERTY(int currentPlatformIndex
               READ currentPlatformIndex WRITE setCurrentPlatformIndex
               RESET resetPlatformIndex
               NOTIFY currentPlatformIndexChanged)
    Q_PROPERTY(Model::Platform* currentPlatform
               READ currentPlatform
               NOTIFY currentPlatformChanged)
    // shortcut for currentPlatform.currentGame
    Q_PROPERTY(Model::Game* currentGame
               READ currentGame
               NOTIFY currentGameChanged)

    // subcomponents
    Q_PROPERTY(ApiParts::Filters* filters READ filters CONSTANT)
    Q_PROPERTY(ApiParts::Meta* meta READ meta CONSTANT)
    Q_PROPERTY(ApiParts::Settings* settings READ settings CONSTANT)
    Q_PROPERTY(ApiParts::System* system READ system CONSTANT)

    // internal properties
    Q_PROPERTY(QString tr READ emptyString NOTIFY languageChanged)

public:
    explicit ApiObject(QObject* parent = nullptr);

    void startScanning() { m_platforms.startScanning(); }

    // platform-related properties

    QQmlListProperty<Model::Platform> getPlatformsProp();
    int currentPlatformIndex() const { return m_platforms.currentIndex(); }
    void setCurrentPlatformIndex(int idx) { m_platforms.setIndex(idx); }
    void resetPlatformIndex() { m_platforms.resetIndex(); }

    Model::Platform* currentPlatform() const { return m_platforms.currentPlatform(); }
    Model::Game* currentGame() const {
        return currentPlatform() ? currentPlatform()->currentGame() : nullptr;
    }

    // game launching
    Q_INVOKABLE void launchGame();

    // subcomponents
    ApiParts::Filters* filters() { return &m_filters; }
    ApiParts::Meta* meta() { return &m_meta; }
    ApiParts::Settings* settings() { return &m_settings; }
    ApiParts::System* system() { return &m_system; }

signals:
    // platform-related properties
    void platformModelChanged();
    void currentPlatformIndexChanged();
    void currentPlatformChanged();
    void currentGameChanged();

    // game launching
    void prepareLaunch();
    void executeLaunch(const Model::Platform*, const Model::Game*);
    void restoreAfterGame(ApiObject*);

    // triggers translation update
    void languageChanged();

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
