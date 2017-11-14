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


#pragma once

#include "model/LocaleList.h"
#include "model/ThemeList.h"

#include <QObject>


namespace Api {

/// Provides a settings interface for the frontend layer
class Settings : public QObject {
    Q_OBJECT

    Q_PROPERTY(bool fullscreen
               READ fullscreen
               WRITE setFullscreen
               NOTIFY fullscreenChanged)
    Q_PROPERTY(Api::LocaleList* locales READ localesPtr CONSTANT)
    Q_PROPERTY(Api::ThemeList* themes READ themesPtr CONSTANT)

public:
    explicit Settings(QObject* parent = nullptr);

    bool fullscreen() const { return m_fullscreen; }
    void setFullscreen(bool);

    LocaleList* localesPtr() { return &m_locales; }
    ThemeList* themesPtr() { return &m_themes; }

signals:
    void fullscreenChanged();

private slots:
    void callScripts();

private:
    bool m_fullscreen;

    LocaleList m_locales;
    ThemeList m_themes;
};

} // namespace Api
