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


package org.pegasus_frontend.android;


final class BatteryInfo {
    private final boolean m_present;
    private final boolean m_plugged;
    private final boolean m_charged;
    private final float m_percent;

    public BatteryInfo(boolean present, boolean plugged, boolean charged, float percent) {
        m_present = present;
        m_plugged = plugged;
        m_charged = charged;
        m_percent = percent;
    }

    public boolean present() { return m_present; }
    public boolean plugged() { return m_plugged; }
    public boolean charged() { return m_charged; }
    public float percent() { return m_percent; }
}
