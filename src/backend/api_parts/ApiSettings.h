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

#include "model/Language.h"
#include "model/ThemeEntry.h"

#include <QObject>
#include <QQmlListProperty>
#include <QTranslator>


namespace ApiParts {

/// Provides a settings interface for the frontend layer
class Settings : public QObject {
    Q_OBJECT

    // multilanguage support
    Q_PROPERTY(int languageIndex
               READ languageIndex WRITE setLanguageIndex
               NOTIFY languageChanged)
    Q_PROPERTY(QQmlListProperty<Model::Language> allLanguages
               READ getTranslationsProp CONSTANT)

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
    int languageIndex() const { return m_language_idx; }
    void setLanguageIndex(int idx);
    QQmlListProperty<Model::Language> getTranslationsProp();

    // theme support
    Model::Theme* currentTheme() const { return m_themes.at(m_theme_idx); }
    int themeIndex() const { return m_theme_idx; }
    void setThemeIndex(int idx);
    QQmlListProperty<Model::Theme> getThemesProp();

signals:
    void languageChanged();
    void themeChanged();

private:
    // multilanguage support
    void initLanguages();
    void loadLanguage(const QString& bcp47tag);

    QTranslator m_translator;
    QList<Model::Language*> m_translations;
    int m_language_idx;

    // theme support
    void initThemes();

    QList<Model::Theme*> m_themes;
    int m_theme_idx;

    // internal
    void callScripts() const;
};

} // namespace ApiParts
