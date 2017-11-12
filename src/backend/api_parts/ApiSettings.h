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
#include <QVector>


namespace ApiParts {

/// Provides access to the list of available locales
class LocaleSettings : public QObject {
    Q_OBJECT

    Q_PROPERTY(Model::Locale* current
               READ qt_current
               NOTIFY localeChanged)
    Q_PROPERTY(int index
               READ qt_index
               WRITE qt_setIndex
               NOTIFY localeChanged)
    Q_PROPERTY(QQmlListProperty<Model::Locale> all
               READ qt_getListProp CONSTANT)

public:
    explicit LocaleSettings(QObject* parent = nullptr);

    const Model::Locale* qt_current() const { return &m_locales[m_locale_idx]; }
    int qt_index() const { return static_cast<int>(m_locale_idx); }
    void qt_setIndex(int);

    QQmlListProperty<Model::Locale> qt_getListProp();


    const Model::Locale& current() const { return m_locales[m_locale_idx]; }

signals:
    void localeChanged();

private:
    std::vector<Model::Locale> m_locales;
    size_t m_locale_idx;

    QTranslator m_translator;

    void selectPreferredLocale();
    void loadSelectedLocale();
    size_t indexOfLocale(const QString&) const;
};


class ThemeSettings : public QObject {
    Q_OBJECT

    Q_PROPERTY(Model::Theme* current
               READ qt_current
               NOTIFY themeChanged)
    Q_PROPERTY(int index
               READ qt_index
               WRITE qt_setIndex
               NOTIFY themeChanged)
    Q_PROPERTY(QQmlListProperty<Model::Theme> all
               READ qt_getListProp CONSTANT)

public:
    explicit ThemeSettings(QObject* parent);

    const Model::Theme* qt_current() const { return &m_themes[m_theme_idx]; }
    int qt_index() const { return static_cast<int>(m_theme_idx); }
    void qt_setIndex(int);

    QQmlListProperty<Model::Theme> qt_getListProp();


    const Model::Theme& current() const { return m_themes[m_theme_idx]; }

signals:
    void themeChanged();

private:
    std::vector<Model::Theme> m_themes;
    size_t m_theme_idx;

    void selectPreferredTheme();
    void printChangeMsg() const;
    size_t indexOfTheme(const QString&) const;
};


/// Provides a settings interface for the frontend layer
class Settings : public QObject {
    Q_OBJECT

    Q_PROPERTY(bool fullscreen
               READ fullscreen WRITE setFullscreen
               NOTIFY fullscreenChanged)
    Q_PROPERTY(ApiParts::LocaleSettings* locales READ localesPtr CONSTANT)
    Q_PROPERTY(ApiParts::ThemeSettings* themes READ themesPtr CONSTANT)

public:
    explicit Settings(QObject* parent = nullptr);

    bool fullscreen() const { return m_fullscreen; }
    void setFullscreen(bool);

    LocaleSettings* localesPtr() { return &m_locales; }
    ThemeSettings* themesPtr() { return &m_themes; }

signals:
    void fullscreenChanged();

private:
    bool m_fullscreen;

    LocaleSettings m_locales;
    ThemeSettings m_themes;
};

} // namespace ApiParts
