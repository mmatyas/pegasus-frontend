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
               NOTIFY currentPlatformChanged)
    Q_PROPERTY(int currentGameIndex
               READ currentGameIndex WRITE setCurrentGameIndex
               NOTIFY currentGameChanged)

public:
    explicit ApiObject(QObject* parent = nullptr);

    QQmlListProperty<Model::Platform> getPlatformsProp();

    int currentPlatformIndex() const { return m_current_platform_idx; }
    int currentGameIndex() const { return m_current_game_idx; }

    void setCurrentPlatformIndex(int);
    void setCurrentGameIndex(int);

signals:
    void currentPlatformChanged();
    void currentGameChanged();

private:
    QList<Model::Platform*> m_platforms;
    int m_current_platform_idx;
    int m_current_game_idx;
};
