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


#pragma once

#include <QObject>
#include <QTimer>


namespace model {
class DeviceInfo : public QObject {
    Q_OBJECT
    Q_CLASSINFO("RegisterEnumClassesUnscoped", "false")

    Q_PROPERTY(BatteryStatus batteryStatus READ batteryStatus NOTIFY batteryStatusChanged)
    Q_PROPERTY(bool batteryCharging READ batteryCharging NOTIFY batteryStatusChanged)
    Q_PROPERTY(float batteryPercent READ batteryPercent NOTIFY batteryLifeChanged)
    Q_PROPERTY(int batterySeconds READ batterySeconds NOTIFY batteryLifeChanged)

public:
    explicit DeviceInfo(QObject* parent = nullptr);

    enum class BatteryStatus {
        Unknown,
        NoBattery,
        Discharging,
        Charging,
        Charged,
    };
    Q_ENUM(BatteryStatus)

    struct BatteryInfo {
        BatteryStatus status;
        float percent;
        int seconds;
    };

    BatteryStatus batteryStatus() const { return m_battery.status; }
    bool batteryCharging() const {
        return m_battery.status == BatteryStatus::Charging || m_battery.status == BatteryStatus::Charged;
    }
    float batteryPercent() const { return m_battery.percent; }
    int batterySeconds() const { return m_battery.seconds; }

signals:
    void batteryStatusChanged();
    void batteryLifeChanged();

private slots:
    void poll_battery();

private:
    QTimer m_battery_poll;
    BatteryInfo m_battery;
};
} // namespace model
