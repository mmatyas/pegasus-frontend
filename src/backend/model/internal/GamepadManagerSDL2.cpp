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

#include <QDataStream>
#include <QDebug>
#include <QFile>
#include <QStringBuilder>
#include <QVersionNumber>


namespace {
void print_sdl_error()
{
    qCritical().noquote() << "Error reported by SDL2:" << SDL_GetError();
}

QString pretty_idx(int device_idx) {
    return QLatin1Char('#') % QString::number(device_idx);
}

QVersionNumber sdl_version()
{
    SDL_version raw_linked;
    SDL_GetVersion(&raw_linked);
    return QVersionNumber(raw_linked.major, raw_linked.minor, raw_linked.patch);
}

QLatin1String gamepaddb_file_suffix(const QVersionNumber& linked_ver)
{
    if (QVersionNumber(2, 0, 9) <= linked_ver)
        return QLatin1String("209");

    if (QVersionNumber(2, 0, 5) <= linked_ver)
        return QLatin1String("205");

    return QLatin1String("204");
}

bool load_gamepaddb(const QVersionNumber& linked_ver)
{
    const QString path = QLatin1String(":/sdl2/gamecontrollerdb_")
        % gamepaddb_file_suffix(linked_ver)
        % QLatin1String(".txt");
    QFile dbfile(path);
    dbfile.open(QFile::ReadOnly);
    Q_ASSERT(dbfile.isOpen()); // it's embedded

    const auto size = static_cast<int>(dbfile.size());
    QByteArray contents(size, 0);
    QDataStream stream(&dbfile);
    stream.readRawData(contents.data(), size);

    SDL_RWops* const rw = SDL_RWFromConstMem(contents.constData(), contents.size());
    if (!rw)
        return false;

    const int entry_cnt = SDL_GameControllerAddMappingsFromRW(rw, 1);
    if (entry_cnt < 0)
        return false;

    return true;
}

void try_register_default_mapping(int device_idx)
{
    std::array<char, 33> guid_raw_str;
    const SDL_JoystickGUID guid = SDL_JoystickGetDeviceGUID(device_idx);
    SDL_JoystickGetGUIDString(guid, guid_raw_str.data(), guid_raw_str.size());

    // concatenation doesn't work with QLatin1Strings...
    const std::string guid_str(guid_raw_str.data());
    const std::string name(SDL_JoystickNameForIndex(device_idx));
    constexpr auto default_mapping("," // emscripten default
        "a:b0,b:b1,x:b2,y:b3,"
        "dpup:b12,dpdown:b13,dpleft:b14,dpright:b15,"
        "leftshoulder:b4,rightshoulder:b5,lefttrigger:b6,righttrigger:b7,"
        "back:b8,start:b9,guide:b16,"
        "leftstick:b10,rightstick:b11,"
        "leftx:a0,lefty:a1,rightx:a2,righty:a3");
    const std::string new_mapping = guid_str + ',' + name + default_mapping;

    if (SDL_GameControllerAddMapping(new_mapping.data()) < 0) {
        qCritical().noquote() << "SDL2: failed to set the default layout for gamepad" << pretty_idx(device_idx);
        print_sdl_error();
        return;
    }
    qInfo().noquote() << "SDL2: using default layout for gamepad" << pretty_idx(device_idx);
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
    const QVersionNumber sdl_ver = sdl_version();
    qInfo().noquote() << "SDL2 version" << sdl_ver;

    if (SDL_Init(SDL_INIT_GAMECONTROLLER) != 0) {
        qCritical().noquote() << "Failed to initialize SDL2. Gamepad support may not work.";
        print_sdl_error();
        return;
    }

    if (!load_gamepaddb(sdl_ver))
        print_sdl_error();

    m_poll_timer.start(16);
}

GamepadManagerSDL2::~GamepadManagerSDL2()
{
    m_poll_timer.stop();
    m_idx_to_device.clear();
    SDL_Quit();
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
        qCritical().noquote() << "SDL2: could not open gamepad" << pretty_idx(device_idx);
        print_sdl_error();
        return;
    }

    const QLatin1String mapping(SDL_GameControllerMapping(pad));
    if (!mapping.isEmpty()) {
        qInfo().noquote().nospace()
            << "SDL2: layout for gamepad " << pretty_idx(device_idx) << " set to `" << mapping << '`';
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
