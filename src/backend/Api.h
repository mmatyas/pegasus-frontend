#pragma once

#include "Model.h"

#include <QObject>
#include <QQmlListProperty>


/// A wrapper class for QML that contains every public property of the backend
class ApiObject : public QObject {
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<Model::Platform> platforms
               READ getPlatformsProp CONSTANT)
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

public:
    explicit ApiObject(QObject* parent = nullptr);

    QQmlListProperty<Model::Platform> getPlatformsProp();

    int currentPlatformIndex() const { return m_current_platform_idx; }
    void setCurrentPlatformIndex(int);
    void resetPlatformIndex();

    int currentGameIndex() const { return m_current_game_idx; }
    void setCurrentGameIndex(int);
    void resetGameIndex();

    Q_INVOKABLE void launchGame();

signals:
    void currentPlatformIndexChanged();
    void currentGameIndexChanged();
    void currentPlatformChanged();
    void currentGameChanged();

    void requestLaunch();
    void executeCommand(ApiObject*, QString);

public slots:
    void onReadyToLaunch();

private:
    QList<Model::Platform*> m_platforms;
    int m_current_platform_idx;
    int m_current_game_idx;

    Model::Platform* m_current_platform;
    Model::Game* m_current_game;

    void findPlatforms();
    void findPlatformGames(Model::Platform* platform);
    void removeDamagedPlatforms();
    void removeEmptyPlatforms();
    void findMetadata(Model::Platform* platform);
    void findGameAssets(const Model::Platform*, Model::Game*);
};
