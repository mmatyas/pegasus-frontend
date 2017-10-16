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
               READ current
               NOTIFY localeChanged)
    Q_PROPERTY(int index
               READ index WRITE setIndex
               NOTIFY localeChanged)
    Q_PROPERTY(QQmlListProperty<Model::Locale> all
               READ getListProp CONSTANT)

public:
    explicit LocaleSettings(QObject* parent = nullptr);

    Model::Locale* current() const { return m_locales.at(index()); }
    int index() const { return m_locale_idx; }
    void setIndex(int idx);

    QQmlListProperty<Model::Locale> getListProp();

signals:
    void localeChanged();

private:
    QVector<Model::Locale*> m_locales;
    int m_locale_idx;

    QTranslator m_translator;

    void selectPreferredLocale();
    void loadSelectedLocale();
    int indexOfLocale(const QString& tag) const;
};


class ThemeSettings : public QObject {
    Q_OBJECT

    Q_PROPERTY(Model::Theme* current
               READ current
               NOTIFY themeChanged)
    Q_PROPERTY(int index
               READ index WRITE setIndex
               NOTIFY themeChanged)
    Q_PROPERTY(QQmlListProperty<Model::Theme> all
               READ getListProp CONSTANT)

public:
    explicit ThemeSettings(QObject* parent);

    Model::Theme* current() const { return m_themes.at(index()); }
    int index() const { return m_theme_idx; }
    void setIndex(int idx);
    QQmlListProperty<Model::Theme> getListProp();

signals:
    void themeChanged();

private:
    QVector<Model::Theme*> m_themes;
    int m_theme_idx;

    void selectPreferredTheme();
    void printChangeMsg() const;
    int indexOfTheme(const QString&) const;
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
