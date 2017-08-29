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

#include "model/Locale.h"
#include "model/ThemeEntry.h"

#include <QObject>
#include <QQmlListProperty>
#include <QTranslator>


namespace ApiParts {

/// Provides a settings interface for the frontend layer
class Settings : public QObject {
    Q_OBJECT

    // multilanguage support
    Q_PROPERTY(Model::Locale* currentLocale
               READ currentLocale
               NOTIFY localeChanged)
    Q_PROPERTY(int localeIndex
               READ localeIndex WRITE setLocaleIndex
               NOTIFY localeChanged)
    Q_PROPERTY(QQmlListProperty<Model::Locale> allLocales
               READ getLocalesProp CONSTANT)

    // theme support
    Q_PROPERTY(Model::Theme* currentTheme
               READ currentTheme
               NOTIFY themeChanged)
    Q_PROPERTY(int themeIndex
               READ themeIndex WRITE setThemeIndex
               NOTIFY themeChanged)
    Q_PROPERTY(QQmlListProperty<Model::Theme> allThemes
               READ getThemesProp CONSTANT)

public:
    explicit Settings(QObject* parent = nullptr);

    // multilanguage support
    Model::Locale* currentLocale() const { return m_locales.at(localeIndex()); }
    int localeIndex() const { return m_locale_idx; }
    void setLocaleIndex(int idx);
    QQmlListProperty<Model::Locale> getLocalesProp();

    // theme support
    Model::Theme* currentTheme() const { return m_themes.at(themeIndex()); }
    int themeIndex() const { return m_theme_idx; }
    void setThemeIndex(int idx);
    QQmlListProperty<Model::Theme> getThemesProp();

signals:
    void localeChanged();
    void themeChanged();

private:
    // multilanguage support

    QTranslator m_translator;
    QList<Model::Locale*> m_locales;
    int m_locale_idx;

    void findAvailableLocales();
    void findPreferredLocale();
    void loadSelectedLocale();
    int indexOfLocale(const QString& tag) const;

    // theme support

    void initThemes();

    QList<Model::Theme*> m_themes;
    int m_theme_idx;

    // internal
    void callScripts() const;
};

} // namespace ApiParts
