// Pegasus Frontend
// Copyright (C) 2017  Mátyás Mustoha
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


#include "Settings.h"

#include "ScriptRunner.h"

#include <QSettings>


namespace {

const QLatin1String SETTINGSKEY_FULLSCREEN("fullscreen");

} // namespace


namespace Types {

Settings::Settings(QObject* parent)
    : QObject(parent)
    , m_locales(this)
    , m_themes(this)
{
    m_fullscreen = QSettings().value(SETTINGSKEY_FULLSCREEN, true).toBool();

    connect(&m_locales, &LocaleList::localeChanged,
            this, &Settings::callScripts);
    connect(&m_themes, &ThemeList::themeChanged,
            this, &Settings::callScripts);

    connect(this, &Settings::fullscreenChanged,
            this, &Settings::callScripts);
}

void Settings::setFullscreen(bool new_val)
{
    if (new_val != m_fullscreen) {
        m_fullscreen = new_val;

        QSettings().setValue(SETTINGSKEY_FULLSCREEN, m_fullscreen);

        emit fullscreenChanged();
    }
}

void Settings::callScripts()
{
    using ScriptEvent = ScriptRunner::EventType;
    ScriptRunner::findAndRunScripts(ScriptEvent::CONFIG_CHANGED);
    ScriptRunner::findAndRunScripts(ScriptEvent::SETTINGS_CHANGED);
}

} // namespace Types
