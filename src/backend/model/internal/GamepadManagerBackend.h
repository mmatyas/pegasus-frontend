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

#include "CliArgs.h"
#include "types/GamepadKeyId.h"

#include <QObject>
#include <QString>


namespace model {

class GamepadManagerBackend : public QObject {
    Q_OBJECT

public:
    GamepadManagerBackend(QObject* parent = nullptr);
    virtual ~GamepadManagerBackend() = default;

    virtual void start(const backend::CliArgs&) {}

    virtual void start_recording(int, GamepadButton) {}
    virtual void start_recording(int, GamepadAxis) {}
    virtual void cancel_recording() {}

signals:
    void connected(int, QString);
    void disconnected(int);
    void nameChanged(int, QString);

    void buttonChanged(int, GamepadButton, bool);
    void axisChanged(int, GamepadAxis, double);

    void buttonConfigured(int, GamepadButton);
    void axisConfigured(int, GamepadAxis);
    void configurationCanceled(int);
};

} // namespace model

