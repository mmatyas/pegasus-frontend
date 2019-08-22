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


#include "GamepadManagerSDL2.h"

#include "LocaleUtils.h"
#include "Log.h"

#include <QDebug>


namespace {
void print_sdl_error()
{
    qCritical().noquote() << "Error reported by SDL2:" << SDL_GetError();
}

void try_register_default_mapping(int device_idx)
{
    std::array<char, 33> guid_raw_str;
    const SDL_JoystickGUID guid = SDL_JoystickGetDeviceGUID(device_idx);
    SDL_JoystickGetGUIDString(guid, guid_raw_str.data(), guid_raw_str.size());

    // concatenation doesn't work with QLatin1Strings...
    const std::string guid_str(guid_raw_str.data());
    const std::string name(SDL_JoystickNameForIndex(device_idx));
    constexpr auto default_mapping(","
        "a:b0,b:b1,back:b8,dpdown:b13,dpleft:b14,dpright:b15,dpup:b12,"
        "guide:b16,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,"
        "rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,"
        "start:b9,x:b2,y:b3");
    const std::string new_mapping = guid_str + ',' + name + default_mapping;

    if (SDL_GameControllerAddMapping(new_mapping.data()) < 0) {
        qCritical().noquote() << "SDL2: failed to add default mapping to joystick.";
        print_sdl_error();
        return;
    }
    qInfo().noquote() << "SDL2: set default mapping for joystick" << guid_raw_str.data();
}

GamepadButton translate_button(Uint8 button)
{
#define GEN(from, to) case SDL_CONTROLLER_BUTTON_##from: return GamepadButton::to
    switch (button) {
        GEN(DPAD_UP, UP);
        GEN(DPAD_DOWN, DOWN);
        GEN(DPAD_LEFT, LEFT);
        GEN(DPAD_RIGHT, RIGHT);
        GEN(A, SOUTH);
        GEN(B, EAST);
        GEN(X, WEST);
        GEN(Y, NORTH);
        GEN(LEFTSHOULDER, L1);
        GEN(LEFTSTICK, L3);
        GEN(RIGHTSHOULDER, R1);
        GEN(RIGHTSTICK, R3);
        GEN(BACK, SELECT);
        GEN(START, START);
        GEN(GUIDE, GUIDE);
        default:
            return GamepadButton::INVALID;
    }
#undef GEN
}

GamepadAxis translate_axis(Uint8 axis)
{
#define GEN(from, to) case SDL_CONTROLLER_AXIS_##from: return GamepadAxis::to
    switch (axis) {
        GEN(LEFTX, LEFTX);
        GEN(LEFTY, LEFTY);
        GEN(RIGHTX, RIGHTX);
        GEN(RIGHTY, RIGHTY);
        default:
            return GamepadAxis::INVALID;
    }
#undef GEN
}

GamepadButton detect_trigger_axis(Uint8 axis)
{
    switch (axis) {
        case SDL_CONTROLLER_AXIS_TRIGGERLEFT: return GamepadButton::L2;
        case SDL_CONTROLLER_AXIS_TRIGGERRIGHT: return GamepadButton::R2;
        default: return GamepadButton::INVALID;
    }
}
} // namespace


namespace model {

GamepadManagerSDL2::GamepadManagerSDL2(QObject* parent)
    : GamepadManagerBackend(parent)
{
    connect(&m_poll_timer, &QTimer::timeout, this, &GamepadManagerSDL2::poll);
}

void GamepadManagerSDL2::start()
{
    if (SDL_Init(SDL_INIT_GAMECONTROLLER) != 0) {
        qCritical().noquote() << "Failed to initialize SDL2. Gamepad support may not work.";
        print_sdl_error();
        return;
    }

    m_poll_timer.start(16);
}

GamepadManagerSDL2::~GamepadManagerSDL2()
{
    m_poll_timer.stop();
    SDL_QuitSubSystem(SDL_INIT_GAMECONTROLLER);
}

void GamepadManagerSDL2::poll()
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_CONTROLLERDEVICEADDED:
                // ignored in favor of SDL_JOYDEVICEADDED
                break;
            case SDL_CONTROLLERDEVICEREMOVED:
                remove_pad_by_iid(event.cdevice.which);
                break;
            case SDL_JOYDEVICEADDED:
                add_controller_by_idx(event.jdevice.which);
                break;
            case SDL_JOYDEVICEREMOVED:
                // ignored in favor of SDL_CONTROLLERDEVICEREMOVED
                break;
            case SDL_CONTROLLERBUTTONUP:
            case SDL_CONTROLLERBUTTONDOWN:
                fwd_button_event(event.cbutton.which, event.cbutton.button, event.cbutton.state == SDL_PRESSED);
                break;
            case SDL_CONTROLLERAXISMOTION:
                fwd_axis_event(event.caxis.which, event.caxis.axis, event.caxis.value);
                break;
            default:
                break;
        }
    }
}

void GamepadManagerSDL2::add_controller_by_idx(int device_idx)
{
    Q_ASSERT(m_idx_to_device.count(device_idx) == 0);

    if (!SDL_IsGameController(device_idx))
        try_register_default_mapping(device_idx);

    SDL_GameController* const pad = SDL_GameControllerOpen(device_idx);
    if (!pad) {
        qCritical().noquote() << "SDL2: could not open gamepad";
        print_sdl_error();
        return;
    }

    QString name = QLatin1String(SDL_GameControllerName(pad)).trimmed();

    SDL_Joystick* const joystick = SDL_GameControllerGetJoystick(pad);
    const SDL_JoystickID iid = SDL_JoystickInstanceID(joystick);

    m_idx_to_device.emplace(device_idx, device_ptr(pad, SDL_GameControllerClose));
    m_iid_to_idx.emplace(iid, device_idx);

    emit connected(device_idx, name);
}

void GamepadManagerSDL2::remove_pad_by_iid(SDL_JoystickID instance_id)
{
    Q_ASSERT(m_iid_to_idx.count(instance_id) == 1);
    Q_ASSERT(m_idx_to_device.count(m_iid_to_idx.at(instance_id)) == 1);

    const int device_idx = m_iid_to_idx.at(instance_id);
    m_idx_to_device.erase(device_idx);
    m_iid_to_idx.erase(instance_id);

    emit disconnected(device_idx);
}

void GamepadManagerSDL2::fwd_button_event(SDL_JoystickID instance_id, Uint8 button, bool pressed)
{
    const int device_idx = m_iid_to_idx.at(instance_id);
    emit buttonChanged(device_idx, translate_button(button), pressed);
}

void GamepadManagerSDL2::fwd_axis_event(SDL_JoystickID instance_id, Uint8 axis, Sint16 value)
{
    const int device_idx = m_iid_to_idx.at(instance_id);

    const GamepadButton button = detect_trigger_axis(axis);
    if (button != GamepadButton::INVALID) {
        emit buttonChanged(device_idx, button, value != 0);
        return;
    }

    const double dblval = value / static_cast<double>(std::numeric_limits<Sint16>::max());
    emit axisChanged(device_idx, translate_axis(axis), dblval);
}

} // namespace model
