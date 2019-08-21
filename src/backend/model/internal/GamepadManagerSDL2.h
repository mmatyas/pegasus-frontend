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

#include "utils/HashMap.h"
#include "GamepadManagerBackend.h"

#include <SDL.h>
#include <QTimer>
#include <memory>


namespace model {

class GamepadManagerSDL2 : public GamepadManagerBackend {
public:
    explicit GamepadManagerSDL2(QObject* parent);
    ~GamepadManagerSDL2();

    void start() final;

private slots:
    void poll();

private:
    QTimer m_poll_timer;

    using device_deleter = void(*)(SDL_GameController*);
    using device_ptr = std::unique_ptr<SDL_GameController, device_deleter>;
    HashMap<int, const device_ptr> m_idx_to_device;
    HashMap<SDL_JoystickID, const int> m_iid_to_idx;

    void add_controller_by_idx(int);
    void remove_pad_by_iid(SDL_JoystickID);
};

} // namespace model
