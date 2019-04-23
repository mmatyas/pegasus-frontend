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


#include "GamepadButtonNavigation.h"

#include "types/GamepadKeyId.h"

#include <QGuiApplication>
#include <QKeyEvent>
#include <QWindow>

using QGamepadButton = QGamepadManager::GamepadButton;


namespace {
static constexpr int KEYDELAY_FIRST = 500;
static constexpr int KEYDELAY_REPEAT = 50;

void emit_key(Qt::Key key, QEvent::Type event_type, bool autorep)
{
    Q_ASSERT(key != Qt::Key_unknown);
    Q_ASSERT(event_type == QEvent::KeyPress || event_type == QEvent::KeyRelease);

    QWindow* const focus_window = qApp ? qApp->focusWindow() : nullptr;
    if (focus_window) {
        QKeyEvent event(event_type, key, Qt::NoModifier, QString(), autorep);
        QGuiApplication::sendEvent(focus_window, &event);
    }
}
} // namespace

GamepadButtonNavigation::GamepadButtonNavigation(QObject* parent)
    : QObject(parent)
    , m_timers {
        { QGamepadButton::ButtonUp, new QTimer(this) },
        { QGamepadButton::ButtonDown, new QTimer(this) },
        { QGamepadButton::ButtonLeft, new QTimer(this) },
        { QGamepadButton::ButtonRight, new QTimer(this) },
        { QGamepadButton::ButtonA, new QTimer(this) },
        { QGamepadButton::ButtonB, new QTimer(this) },
        { QGamepadButton::ButtonX, new QTimer(this) },
        { QGamepadButton::ButtonY, new QTimer(this) },
        { QGamepadButton::ButtonL1, new QTimer(this) },
        { QGamepadButton::ButtonL2, new QTimer(this) },
        { QGamepadButton::ButtonL3, new QTimer(this) },
        { QGamepadButton::ButtonR1, new QTimer(this) },
        { QGamepadButton::ButtonR2, new QTimer(this) },
        { QGamepadButton::ButtonR3, new QTimer(this) },
        { QGamepadButton::ButtonSelect, new QTimer(this) },
        { QGamepadButton::ButtonStart, new QTimer(this) },
        { QGamepadButton::ButtonGuide, new QTimer(this) },
    }
    , m_keys {
        { QGamepadButton::ButtonUp, Qt::Key_Up },
        { QGamepadButton::ButtonDown, Qt::Key_Down },
        { QGamepadButton::ButtonLeft, Qt::Key_Left },
        { QGamepadButton::ButtonRight, Qt::Key_Right },
        { QGamepadButton::ButtonA, static_cast<Qt::Key>(GamepadKeyId::A) },
        { QGamepadButton::ButtonB, static_cast<Qt::Key>(GamepadKeyId::B) },
        { QGamepadButton::ButtonX, static_cast<Qt::Key>(GamepadKeyId::X) },
        { QGamepadButton::ButtonY, static_cast<Qt::Key>(GamepadKeyId::Y) },
        { QGamepadButton::ButtonL1, static_cast<Qt::Key>(GamepadKeyId::L1) },
        { QGamepadButton::ButtonL2, static_cast<Qt::Key>(GamepadKeyId::L2) },
        { QGamepadButton::ButtonL3, static_cast<Qt::Key>(GamepadKeyId::L3) },
        { QGamepadButton::ButtonR1, static_cast<Qt::Key>(GamepadKeyId::R1) },
        { QGamepadButton::ButtonR2, static_cast<Qt::Key>(GamepadKeyId::R2) },
        { QGamepadButton::ButtonR3, static_cast<Qt::Key>(GamepadKeyId::R3) },
        { QGamepadButton::ButtonSelect, static_cast<Qt::Key>(GamepadKeyId::SELECT) },
        { QGamepadButton::ButtonStart, static_cast<Qt::Key>(GamepadKeyId::START) },
        { QGamepadButton::ButtonGuide, static_cast<Qt::Key>(GamepadKeyId::GUIDE) },
    }
{
    for (const auto& pair : m_timers) {
        connect(pair.second, &QTimer::timeout,
                this, &GamepadButtonNavigation::onTimerTimeout);
    }
}

void GamepadButtonNavigation::onButtonPress(int, QGamepadManager::GamepadButton button)
{
    const auto it = m_timers.find(button);
    if (it == m_timers.cend())
        return;

    emit_key(m_keys.at(it->first), QEvent::KeyPress, false);
    it->second->start(KEYDELAY_FIRST);
}

void GamepadButtonNavigation::onButtonRelease(int, QGamepadManager::GamepadButton button)
{
    const auto it = m_timers.find(button);
    if (it == m_timers.cend())
        return;

    emit_key(m_keys.at(it->first), QEvent::KeyRelease, false);
    it->second->stop();
}

void GamepadButtonNavigation::onTimerTimeout()
{
    auto timer = static_cast<QTimer* const>(QObject::sender());

    using HashEntry = const decltype(m_timers)::value_type;
    const auto it = std::find_if(m_timers.cbegin(), m_timers.cend(),
        [timer](HashEntry& pair){ return pair.second == timer; });
    if (it == m_timers.cend())
        return;

    const Qt::Key key = m_keys.at(it->first);
        emit_key(key, QEvent::KeyRelease, true);
        emit_key(key, QEvent::KeyPress, true);
    timer->start(KEYDELAY_REPEAT);
}
