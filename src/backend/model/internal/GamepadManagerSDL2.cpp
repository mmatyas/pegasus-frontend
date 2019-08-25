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
#include "utils/StdStringHelpers.h"

#include <QDataStream>
#include <QDebug>
#include <QFile>
#include <QStringBuilder>


namespace {
constexpr uint16_t version(uint16_t major, uint16_t minor, uint16_t micro)
{
    return major * 1000u + minor * 100u + micro;
}

std::unique_ptr<char, void(*)(void*)> freeable_str(char* const str)
{
    return { str, SDL_free };
}

void print_sdl_error()
{
    qCritical().noquote() << "Error reported by SDL2:" << SDL_GetError();
}

QString pretty_idx(int device_idx) {
    return QLatin1Char('#') % QString::number(device_idx);
}

uint16_t linked_sdl_version()
{
    SDL_version raw;
    SDL_GetVersion(&raw);
    return version(raw.major, raw.minor, raw.patch);
}

void print_sdl_version()
{
    SDL_version raw;
    SDL_GetVersion(&raw);
    qInfo().noquote().nospace()
        << "SDL version " << raw.major << '.' << raw.minor << '.' << raw.patch;
}

QLatin1String gamepaddb_file_suffix(uint16_t linked_ver)
{
    if (version(2, 0, 9) <= linked_ver)
        return QLatin1String("209");

    if (version(2, 0, 5) <= linked_ver)
        return QLatin1String("205");

    return QLatin1String("204");
}

bool load_gamepaddb(uint16_t linked_ver)
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
    const auto guid_str = QLatin1String(guid_raw_str.data()).trimmed();
    const auto name = QLatin1String(SDL_JoystickNameForIndex(device_idx));
    constexpr auto default_mapping("," // emscripten default
        "a:b0,b:b1,x:b2,y:b3,"
        "dpup:b12,dpdown:b13,dpleft:b14,dpright:b15,"
        "leftshoulder:b4,rightshoulder:b5,lefttrigger:b6,righttrigger:b7,"
        "back:b8,start:b9,guide:b16,"
        "leftstick:b10,rightstick:b11,"
        "leftx:a0,lefty:a1,rightx:a2,righty:a3");
    const QString new_mapping = guid_str % QLatin1Char(',') % name % default_mapping;

    if (SDL_GameControllerAddMapping(new_mapping.toLocal8Bit().data()) < 0) {
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

const char* to_fieldname(GamepadButton button)
{
#define GEN(from, to) case GamepadButton::from: return SDL_GameControllerGetStringForButton(SDL_CONTROLLER_BUTTON_##to);
    switch (button) {
        GEN(UP, DPAD_UP);
        GEN(DOWN, DPAD_DOWN);
        GEN(LEFT, DPAD_LEFT);
        GEN(RIGHT, DPAD_RIGHT);
        GEN(SOUTH, A);
        GEN(EAST, B);
        GEN(WEST, X);
        GEN(NORTH, Y);
        GEN(L1, LEFTSHOULDER);
        GEN(L3, LEFTSTICK);
        GEN(R1, RIGHTSHOULDER);
        GEN(R3, RIGHTSTICK);
        GEN(SELECT, BACK);
        GEN(START, START);
        GEN(GUIDE, GUIDE);
        case GamepadButton::L2:
            return SDL_GameControllerGetStringForAxis(SDL_CONTROLLER_AXIS_TRIGGERLEFT);
        case GamepadButton::R2:
            return SDL_GameControllerGetStringForAxis(SDL_CONTROLLER_AXIS_TRIGGERRIGHT);
        default:
            Q_UNREACHABLE();
            return nullptr;
    }
#undef GEN
}

const char* to_fieldname(GamepadAxis axis)
{
#define GEN(from, to) case GamepadAxis::from: return SDL_GameControllerGetStringForAxis(SDL_CONTROLLER_AXIS_##to);
    switch (axis) {
        GEN(LEFTX, LEFTX);
        GEN(LEFTY, LEFTY);
        GEN(RIGHTX, RIGHTX);
        GEN(RIGHTY, RIGHTY);
        default:
            Q_UNREACHABLE();
            return nullptr;
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

std::string generate_hat_str(int hat_idx, int hat_value)
{
    return 'h' + std::to_string(hat_idx) + '.' + std::to_string(hat_value);
}
std::string generate_axis_str(int axis_idx)
{
    return 'a' + std::to_string(axis_idx);
}
std::string generate_button_str(int button_idx)
{
    return 'b' + std::to_string(button_idx);
}
std::string generate_binding_str(const SDL_GameControllerButtonBind& bind)
{
    switch (bind.bindType) {
        case SDL_CONTROLLER_BINDTYPE_BUTTON:
            return generate_button_str(bind.value.button);
        case SDL_CONTROLLER_BINDTYPE_AXIS:
            return generate_axis_str(bind.value.axis);
        case SDL_CONTROLLER_BINDTYPE_HAT:
            return generate_hat_str(bind.value.hat.hat, bind.value.hat.hat_mask);
        default:
            return {};
    }
}
} // namespace


namespace model {

bool GamepadManagerSDL2::RecordingState::is_active() const
{
    return device >= 0;
}
void GamepadManagerSDL2::RecordingState::reset()
{
    device = -1;
    field = nullptr;
    value.clear();
}

GamepadManagerSDL2::GamepadManagerSDL2(QObject* parent)
    : GamepadManagerBackend(parent)
    , m_sdl_version(linked_sdl_version())
{
    connect(&m_poll_timer, &QTimer::timeout, this, &GamepadManagerSDL2::poll);
}

void GamepadManagerSDL2::start()
{
    print_sdl_version();

    if (SDL_Init(SDL_INIT_GAMECONTROLLER) != 0) {
        qCritical().noquote() << "Failed to initialize SDL2. Gamepad support may not work.";
        print_sdl_error();
        return;
    }

    if (!load_gamepaddb(m_sdl_version))
        print_sdl_error();

    m_poll_timer.start(16);
}

GamepadManagerSDL2::~GamepadManagerSDL2()
{
    m_poll_timer.stop();
    m_idx_to_device.clear();
    SDL_Quit();
}

void GamepadManagerSDL2::start_recording(int device_idx, GamepadButton button)
{
    m_recording.reset();
    m_recording.device = device_idx;
    m_recording.field = to_fieldname(button);
    Q_ASSERT(m_recording.field);

    qDebug() << "recording" << m_recording.device << m_recording.field;
}

void GamepadManagerSDL2::start_recording(int device_idx, GamepadAxis axis)
{
    m_recording.reset();
    m_recording.device = device_idx;
    m_recording.field = to_fieldname(axis);
    Q_ASSERT(m_recording.field);

    qDebug() << "recording" << m_recording.device << m_recording.field;
}

void GamepadManagerSDL2::cancel_recording()
{
    if (m_recording.is_active())
        emit configurationCanceled(m_recording.device);

    m_recording.reset();
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
            case SDL_CONTROLLERDEVICEREMAPPED:
                qDebug() << "remapped" << event.cdevice.which;
                break;
            case SDL_JOYDEVICEADDED:
                add_controller_by_idx(event.jdevice.which);
                break;
            case SDL_JOYDEVICEREMOVED:
                // ignored in favor of SDL_CONTROLLERDEVICEREMOVED
                break;
            case SDL_CONTROLLERBUTTONUP:
            case SDL_CONTROLLERBUTTONDOWN:
                // also ignore input from other (non-recording) gamepads
                if (!m_recording.is_active()) {
                    qDebug() << "CBTN";
                    const bool pressed = event.cbutton.state == SDL_PRESSED;
                    fwd_button_event(event.cbutton.which, event.cbutton.button, pressed);
                }
                break;
            case SDL_CONTROLLERAXISMOTION:
                if (!m_recording.is_active())
                    fwd_axis_event(event.caxis.which, event.caxis.axis, event.caxis.value);
                break;
            case SDL_JOYBUTTONUP:
                // ignored
                break;
            case SDL_JOYBUTTONDOWN:
                qDebug() << "JBTN" << event.jbutton.which << event.jbutton.button;
                record_joy_button_maybe(event.jbutton.which, event.jbutton.button);
                break;
            case SDL_JOYHATMOTION:
                record_joy_hat_maybe(event.jhat.which, event.jhat.hat, event.jhat.value);
                break;
            case SDL_JOYAXISMOTION:
                record_joy_axis_maybe(event.jaxis.which, event.jaxis.axis, event.jaxis.value);
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

    const auto mapping = freeable_str(SDL_GameControllerMapping(pad));
    if (!mapping) {
        qInfo().noquote().nospace()
            << "SDL2: layout for gamepad " << pretty_idx(device_idx) << " set to `" << mapping.get() << '`';
    }

    QString name = QLatin1String(SDL_GameControllerName(pad)).trimmed();

    SDL_Joystick* const joystick = SDL_GameControllerGetJoystick(pad);
    const SDL_JoystickID iid = SDL_JoystickInstanceID(joystick);

    m_idx_to_device.emplace(device_idx, device_ptr(pad, SDL_GameControllerClose));
    m_iid_to_idx.emplace(iid, device_idx);

    emit connected(device_idx, name);


    qDebug() << generate_mapping(device_idx).c_str();
}

void GamepadManagerSDL2::remove_pad_by_iid(SDL_JoystickID instance_id)
{
    Q_ASSERT(m_iid_to_idx.count(instance_id) == 1);
    Q_ASSERT(m_idx_to_device.count(m_iid_to_idx.at(instance_id)) == 1);

    const int device_idx = m_iid_to_idx.at(instance_id);
    m_idx_to_device.erase(device_idx);
    m_iid_to_idx.erase(instance_id);

    if (m_recording.device == device_idx)
        cancel_recording();

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

void GamepadManagerSDL2::record_joy_button_maybe(SDL_JoystickID instance_id, Uint8 button)
{
    if (!m_recording.is_active())
        return;

    const int device_idx = m_iid_to_idx.at(instance_id);
    if (m_recording.device != device_idx)
        return;

    m_recording.value = generate_button_str(button);
    qDebug().nospace().noquote() << "REC BTN" << m_recording.field << ':' << m_recording.value.c_str();
    qDebug() << generate_mapping(device_idx).c_str();
    cancel_recording();
}

void GamepadManagerSDL2::record_joy_axis_maybe(SDL_JoystickID instance_id, Uint8 axis, Sint16 axis_value)
{
    if (!m_recording.is_active())
        return;

    const int device_idx = m_iid_to_idx.at(instance_id);
    if (m_recording.device != device_idx)
        return;

    constexpr Sint16 deadzone = std::numeric_limits<Sint16>::max() / 2;
    if (-deadzone < axis_value && axis_value < deadzone)
        return;

    if (axis_value == std::numeric_limits<Sint16>::min()) // some triggers start from negative
        return;

    m_recording.value = generate_axis_str(axis);
    qDebug() << "REC AXIS" << axis << axis_value;
    qDebug().nospace().noquote() << "REC AXIS" << m_recording.field << ':' << m_recording.value.c_str();
    qDebug() << generate_mapping(device_idx).c_str();
    cancel_recording();
}

void GamepadManagerSDL2::record_joy_hat_maybe(SDL_JoystickID instance_id, Uint8 hat, Uint8 hat_value)
{
    if (!m_recording.is_active())
        return;

    const int device_idx = m_iid_to_idx.at(instance_id);
    if (m_recording.device != device_idx)
        return;

    if (hat_value == SDL_HAT_CENTERED)
        return;

    qDebug() << "REC HAT" << hat << hat_value;
    m_recording.value = generate_hat_str(hat, hat_value);
    qDebug().nospace().noquote() << "REC HAT" << m_recording.field << ':' << m_recording.value.c_str();
    qDebug() << generate_mapping(device_idx).c_str();
    cancel_recording();
}

std::string GamepadManagerSDL2::generate_mapping_for_field(const char* const field, const SDL_GameControllerButtonBind& current_bind)
{
    // new mapping
    if (field == m_recording.field)
        return std::string(field) + ':' + m_recording.value;

    // old mapping
    const std::string value = generate_binding_str(current_bind);
    if (value.empty() || value == m_recording.value)
        return {};

    // unaffected mapping
    return std::string(field) + ':' + value;
}

std::string GamepadManagerSDL2::generate_mapping(int device_idx)
{
    Q_ASSERT(m_idx_to_device.count(device_idx) == 1);
    const auto& pad_ptr = m_idx_to_device.at(device_idx);

    std::array<char, 33> guid_raw_str;
    const SDL_JoystickGUID guid = SDL_JoystickGetDeviceGUID(device_idx);
    SDL_JoystickGetGUIDString(guid, guid_raw_str.data(), guid_raw_str.size());

    std::vector<std::string> list;
        list.emplace_back(utils::trimmed(guid_raw_str.data()));
        list.emplace_back(SDL_GameControllerName(pad_ptr.get()));

    for (int btn_idx = 0; btn_idx < SDL_CONTROLLER_BUTTON_MAX; btn_idx++) {
        const auto button = static_cast<SDL_GameControllerButton>(btn_idx);

        const char* const field = SDL_GameControllerGetStringForButton(button);
        const auto current_bind = SDL_GameControllerGetBindForButton(pad_ptr.get(), button);

        std::string mapping = generate_mapping_for_field(field, current_bind);
        if (!mapping.empty())
            list.emplace_back(std::move(mapping));
    }

    for (int axis_idx = 0; axis_idx < SDL_CONTROLLER_AXIS_MAX; axis_idx++) {
        const auto axis = static_cast<SDL_GameControllerAxis>(axis_idx);

        const char* const field = SDL_GameControllerGetStringForAxis(axis);
        const auto current_bind = SDL_GameControllerGetBindForAxis(pad_ptr.get(), axis);

        std::string mapping = generate_mapping_for_field(field, current_bind);
        if (!mapping.empty())
            list.emplace_back(std::move(mapping));
    }

    std::sort(list.begin() + 2, list.end());
    if (version(2, 0, 5) <= m_sdl_version)
        list.emplace_back(std::string("platform:") + SDL_GetPlatform());

    size_t out_len = 0;
    for (const std::string& item : list)
        out_len += item.size() + 1;

    std::string out;
    out.reserve(out_len);
    for (const std::string& item : list)
        out += std::move(item) + ',';

    qDebug() << out.c_str();
    return out;
}

} // namespace model
