// Pegasus Frontend
// Copyright (C) 2017-2018  Mátyás Mustoha
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

#include "types/GamepadKeyId.h"
#include "types/KeyEventType.h"
#include "utils/HashMap.h"
#include "utils/NoCopyNoMove.h"

#include <QString>
#include <QVector>
#include <QKeySequence>
#include <functional>
#include <map>
#include <memory>

namespace providers { class Provider; }


namespace appsettings {

struct General {
    const QString DEFAULT_LOCALE;
    const QString DEFAULT_THEME;

    bool portable;
    bool fullscreen;
    bool mouse_support;
    QString locale;
    QString theme;

    General();
    NO_COPY_NO_MOVE(General)
};


class Keys {
public:
    Keys();
    NO_COPY_NO_MOVE(Keys)

    void add_key(KeyEvent, QKeySequence);
    void del_key(KeyEvent, const QKeySequence&);
    void clear(KeyEvent);
    void resetAll();

    const QVector<QKeySequence>& at(KeyEvent) const;
    const QVector<QKeySequence>& operator[](KeyEvent) const;

private:
    HashMap<KeyEvent, QVector<QKeySequence>, EnumHash> m_event_keymap;
};

} // namespace appsettings


struct AppSettings {
    static appsettings::General general;
    static appsettings::Keys keys;

    static void load_config();
    static void save_config();
    static void parse_gamedirs(const std::function<void(const QString&)>&);

    static const std::map<QKeySequence, QString> gamepadButtonNames;
    static const std::vector<std::unique_ptr<providers::Provider>> providers;
};
