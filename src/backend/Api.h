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
#include "api_parts/ApiMeta.h"
#include "api_parts/ApiSettings.h"
#include "api_parts/ApiSystem.h"

#include <QFutureWatcher>
#include <QObject>
#include <QQmlListProperty>


/// Provides data access for QML
///
/// Provides an API for the frontend layer, to allow accessing every public
/// property of the backend from QML.
class ApiObject : public QObject {
    Q_OBJECT

    Q_PROPERTY(QQmlListProperty<Model::Platform> platforms
               READ getPlatformsProp
               NOTIFY platformModelChanged)
    Q_PROPERTY(int currentPlatformIndex
               READ currentPlatformIndex WRITE setCurrentPlatformIndex
               RESET resetPlatformIndex
               NOTIFY currentPlatformIndexChanged)
    Q_PROPERTY(int currentGameIndex
               READ currentGameIndex WRITE setCurrentGameIndex
               RESET resetGameIndex
               NOTIFY currentGameIndexChanged)
    Q_PROPERTY(Model::Platform* currentPlatform MEMBER m_current_platform
               NOTIFY currentPlatformChanged)
    Q_PROPERTY(Model::Game* currentGame MEMBER m_current_game
               NOTIFY currentGameChanged)

    Q_PROPERTY(ApiParts::Meta* meta READ meta CONSTANT)
    Q_PROPERTY(ApiParts::System* system READ system CONSTANT)
    Q_PROPERTY(ApiParts::Settings* settings READ settings CONSTANT)

public:
    explicit ApiObject(QObject* parent = nullptr);

    QQmlListProperty<Model::Platform> getPlatformsProp() {
        return QQmlListProperty<Model::Platform>(this, m_platforms);
    }

    int currentPlatformIndex() const { return m_current_platform_idx; }
    void setCurrentPlatformIndex(int);
    void resetPlatformIndex();

    int currentGameIndex() const { return m_current_game_idx; }
    void setCurrentGameIndex(int);
    void resetGameIndex();

    Q_INVOKABLE void launchGame();

    ApiParts::Meta* meta() { return &m_meta; }
    ApiParts::System* system() { return &m_system; }
    ApiParts::Settings* settings() { return &m_settings; }

signals:
    // the main data structures
    void platformModelChanged();
    void currentPlatformIndexChanged();
    void currentGameIndexChanged();
    void currentPlatformChanged();
    void currentGameChanged();

    // game launching
    void prepareLaunch();
    void executeLaunch(const Model::Platform*, const Model::Game*);
    void restoreAfterGame(ApiObject*);

public slots:
    // game launch communication
    void onLoadingFinished();
    void onReadyToLaunch();
    void onGameFinished();

private:
    QList<Model::Platform*> m_platforms;

    int m_current_platform_idx;
    int m_current_game_idx;
    Model::Platform* m_current_platform;
    Model::Game* m_current_game;

    ApiParts::Meta m_meta;
    ApiParts::System m_system;
    ApiParts::Settings m_settings;

    // initialization
    QFutureWatcher<void> m_loading_watcher;
};
