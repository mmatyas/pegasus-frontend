// Pegasus Frontend
// Copyright (C) 2017-2019  Mátyás Mustoha
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

#include "Gamepad.h"
#include "GamepadManagerBackend.h"
#include "utils/HashMap.h"

#include "QtQmlTricks/QQmlObjectListModel.h"
#include <QObject>
#include <QString>
#include <QVector>

#ifndef Q_OS_ANDROID
#  include "GamepadAxisNavigation.h"
#  include "GamepadButtonNavigation.h"
#endif


namespace model {

class GamepadManager : public QObject {
    Q_OBJECT

    QML_OBJMODEL_PROPERTY(Gamepad, devices)

public:
    explicit GamepadManager(QObject* parent = nullptr);

signals:
    void connected(int);
    void disconnected(QString);

private slots:
    void bkOnConnected(int);
    void bkOnDisconnected(int);
    void bkOnNameChanged(int, QString);

private:
    GamepadManagerBackend* const m_backend;

#ifndef Q_OS_ANDROID
    GamepadButtonNavigation padbuttonnav;
    GamepadAxisNavigation padaxisnav;
#endif
};

} // namespace model
