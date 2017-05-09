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

    // TODO: move this under api.meta
    // TODO: maybe a state field?
    Q_PROPERTY(bool isInitializing MEMBER m_init_in_progress
               NOTIFY initComplete)
    Q_PROPERTY(QString gitRevision MEMBER m_git_revision CONSTANT)

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

signals:
    // initialization (loading the data files)
    void initComplete();

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

    // initialization
    // TODO: move these to some kind of subclass
    QFutureWatcher<void> m_loading_watcher;
    qint64 m_loading_time_ms;
    bool m_init_in_progress;

    // meta
    const QString m_git_revision;
};
