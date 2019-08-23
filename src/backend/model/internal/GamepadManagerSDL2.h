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
#include <QVersionNumber>
#include <memory>


namespace model {

class GamepadManagerSDL2 : public GamepadManagerBackend {
public:
    explicit GamepadManagerSDL2(QObject* parent);
    ~GamepadManagerSDL2();

    void start() final;

    void start_recording(int, GamepadButton) final;
    void start_recording(int, GamepadAxis) final;
    void cancel_recording() final;

private slots:
    void poll();

private:
    const QVersionNumber m_sdl_version;
    QTimer m_poll_timer;

    using device_deleter = void(*)(SDL_GameController*);
    using device_ptr = std::unique_ptr<SDL_GameController, device_deleter>;
    HashMap<int, const device_ptr> m_idx_to_device;
    HashMap<SDL_JoystickID, const int> m_iid_to_idx;

    void add_controller_by_idx(int);
    void remove_pad_by_iid(SDL_JoystickID);
    void fwd_button_event(SDL_JoystickID, Uint8, bool);
    void fwd_axis_event(SDL_JoystickID, Uint8, Sint16);

    int m_recording_device;
    GamepadButton m_recording_button;
    GamepadAxis m_recording_axis;
    bool is_recording() const;

    void record_joy_button_maybe(SDL_JoystickID, Uint8);
    void record_joy_axis_maybe(SDL_JoystickID, Uint8, Sint16);
    void record_joy_hat_maybe(SDL_JoystickID, Uint8, Uint8);
};

} // namespace model
