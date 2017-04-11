#pragma once

#include "Model.h"

#include <QObject>
#include <QQmlListProperty>


/// A wrapper class for QML that contains every public property of the backend
class ApiObject : public QObject {
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<Model::Platform> platforms READ getPlatformsProp CONSTANT)

public:
    explicit ApiObject(QObject* parent = nullptr);

    QQmlListProperty<Model::Platform> getPlatformsProp();

private:
    QList<Model::Platform*> m_platforms;
};
