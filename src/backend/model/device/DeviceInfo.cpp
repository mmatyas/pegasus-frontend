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
#elif defined(Q_OS_ANDROID)
#include "platform/AndroidHelpers.h"
#include <QtAndroidExtras/QAndroidJniEnvironment>
#include <QtAndroidExtras/QAndroidJniObject>
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
#elif defined(Q_OS_ANDROID)
model::DeviceInfo::BatteryInfo query_battery()
{
    static constexpr auto JNI_METHOD = "queryBattery";
    static constexpr auto JNI_SIGNATURE = "()Lorg/pegasus_frontend/android/BatteryInfo;";

    QAndroidJniEnvironment jni_env;

    const QAndroidJniObject jni_obj = QAndroidJniObject::callStaticObjectMethod(android::jni_classname(), JNI_METHOD, JNI_SIGNATURE);
    if (!jni_obj.isValid())
        return {model::DeviceInfo::BatteryStatus::Unknown, NAN, -1};

    const bool present = jni_obj.callMethod<jboolean>("present");
    const bool plugged = jni_obj.callMethod<jboolean>("plugged");
    const bool charged = jni_obj.callMethod<jboolean>("charged");
    const float percent = qBound(0.f, jni_obj.callMethod<jfloat>("percent"), 1.f);

    if (!present)
        return {model::DeviceInfo::BatteryStatus::NoBattery, NAN, -1};
    if (charged)
        return {model::DeviceInfo::BatteryStatus::Charged, 1.f, -1};
    if (plugged)
        return {model::DeviceInfo::BatteryStatus::Charging, percent, -1};

    return {model::DeviceInfo::BatteryStatus::Discharging, percent, -1};
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
    , m_battery { BatteryStatus::Unknown, NAN, 0 }
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
