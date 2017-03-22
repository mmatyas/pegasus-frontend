#pragma once

#include "Model.h"

#include <QObject>


/// A wrapper class for QML that contains every public property of the backend
class ApiObject : public QObject {
    Q_OBJECT

    Q_PROPERTY(QObject* platforms READ platformsPtr CONSTANT)

public:
    explicit ApiObject(QObject* parent = nullptr);

    Model::PlatformModel* platformsPtr() { return &m_platforms; }

private:
    Model::PlatformModel m_platforms;
};
