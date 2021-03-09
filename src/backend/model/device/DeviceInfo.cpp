// Pegasus Frontend
// Copyright (C) 2017-2021  Mátyás Mustoha
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


#include "DeviceInfo.h"

#ifdef WITH_SDL_POWER
#include <SDL.h>
#endif

#include <cmath>


namespace {
#ifdef WITH_SDL_POWER
model::DeviceInfo::BatteryStatus sdl_state_to_qt(const SDL_PowerState sdl_state)
{
    using Status = model::DeviceInfo::BatteryStatus;
    switch (sdl_state) {
        case SDL_POWERSTATE_NO_BATTERY: return Status::NoBattery;
        case SDL_POWERSTATE_ON_BATTERY: return Status::Discharging;
        case SDL_POWERSTATE_CHARGING: return Status::Charging;
        case SDL_POWERSTATE_CHARGED: return Status::Charged;
        default: return Status::Unknown;
    }
}

model::DeviceInfo::BatteryInfo query_battery()
{
    int sdl_secs = 0;
    int sdl_pct = 0;
    const SDL_PowerState sdl_state = SDL_GetPowerInfo(&sdl_secs, &sdl_pct);

    return {
        sdl_state_to_qt(sdl_state),
        sdl_pct < 0 ? NAN : (sdl_pct / 100.f),
        sdl_secs,
    };
}
#else
model::DeviceInfo::BatteryInfo query_battery()
{
    return {model::DeviceInfo::BatteryStatus::Unknown, NAN, -1};
}
#endif
} // namespace


namespace model {
DeviceInfo::DeviceInfo(QObject* parent)
    : QObject(parent)
{
    poll_battery();
    connect(&m_battery_poll, &QTimer::timeout, this, &DeviceInfo::poll_battery);
    m_battery_poll.start(30 * 1000);
}

void DeviceInfo::poll_battery()
{
    BatteryInfo new_info = query_battery();

    const bool state_changed = m_battery.status != new_info.status;
    const bool value_changed = m_battery.seconds != new_info.seconds
        || !qFuzzyCompare(m_battery.percent, new_info.percent);

    m_battery = std::move(new_info);

    if (state_changed)
        emit batteryStatusChanged();
    if (value_changed)
        emit batteryLifeChanged();
}
} // namespace model
