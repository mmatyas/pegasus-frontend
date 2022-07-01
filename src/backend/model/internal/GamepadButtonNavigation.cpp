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

#include <QGuiApplication>
#include <QKeyEvent>
#include <QTimer>
#include <QWindow>


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
        { GamepadButton::UP, new QTimer(this) },
        { GamepadButton::DOWN, new QTimer(this) },
        { GamepadButton::LEFT, new QTimer(this) },
        { GamepadButton::RIGHT, new QTimer(this) },
        { GamepadButton::NORTH, new QTimer(this) },
        { GamepadButton::SOUTH, new QTimer(this) },
        { GamepadButton::EAST, new QTimer(this) },
        { GamepadButton::WEST, new QTimer(this) },
        { GamepadButton::L1, new QTimer(this) },
        { GamepadButton::L2, new QTimer(this) },
        { GamepadButton::L3, new QTimer(this) },
        { GamepadButton::R1, new QTimer(this) },
        { GamepadButton::R2, new QTimer(this) },
        { GamepadButton::R3, new QTimer(this) },
        { GamepadButton::SELECT, new QTimer(this) },
        { GamepadButton::START, new QTimer(this) },
        { GamepadButton::GUIDE, new QTimer(this) },
    }
    , m_keys {
        { GamepadButton::UP, Qt::Key_Up },
        { GamepadButton::DOWN, Qt::Key_Down },
        { GamepadButton::LEFT, Qt::Key_Left },
        { GamepadButton::RIGHT, Qt::Key_Right },
        { GamepadButton::SOUTH, static_cast<Qt::Key>(GamepadKeyId::A) },
        { GamepadButton::EAST, static_cast<Qt::Key>(GamepadKeyId::B) },
        { GamepadButton::WEST, static_cast<Qt::Key>(GamepadKeyId::X) },
        { GamepadButton::NORTH, static_cast<Qt::Key>(GamepadKeyId::Y) },
        { GamepadButton::L1, static_cast<Qt::Key>(GamepadKeyId::L1) },
        { GamepadButton::L2, static_cast<Qt::Key>(GamepadKeyId::L2) },
        { GamepadButton::L3, static_cast<Qt::Key>(GamepadKeyId::L3) },
        { GamepadButton::R1, static_cast<Qt::Key>(GamepadKeyId::R1) },
        { GamepadButton::R2, static_cast<Qt::Key>(GamepadKeyId::R2) },
        { GamepadButton::R3, static_cast<Qt::Key>(GamepadKeyId::R3) },
        { GamepadButton::SELECT, static_cast<Qt::Key>(GamepadKeyId::SELECT) },
        { GamepadButton::START, static_cast<Qt::Key>(GamepadKeyId::START) },
        { GamepadButton::GUIDE, static_cast<Qt::Key>(GamepadKeyId::GUIDE) },
    }
{
    for (const auto& pair : m_timers) {
        connect(pair.second, &QTimer::timeout,
                this, &GamepadButtonNavigation::onTimerTimeout);
    }
}

void GamepadButtonNavigation::onButtonChanged(int, GamepadButton button, bool pressed)
{
    const auto it = m_timers.find(button);
    if (it == m_timers.cend())
        return;

    const auto event_type = pressed ? QEvent::KeyPress : QEvent::KeyRelease;
    emit_key(m_keys.at(it->first), event_type, false);

    if (pressed)
        it->second->start(KEYDELAY_FIRST);
    else
        it->second->stop();
}

void GamepadButtonNavigation::onTimerTimeout()
{
    QTimer* const timer = static_cast<QTimer*>(QObject::sender());

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
