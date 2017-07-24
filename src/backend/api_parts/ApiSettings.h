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

#include <QObject>
#include <QQmlListProperty>
#include <QTranslator>
#include <QVector>


namespace ApiParts {

/// An utility class to contain language informations
class Language : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString tag MEMBER m_bcp47tag CONSTANT)
    Q_PROPERTY(QString name MEMBER m_name CONSTANT)

public:
    explicit Language(QString bcp47tag, QString name,
                      QObject* parent = nullptr);

    const QString tag() const { return m_bcp47tag; }
    const QString name() const { return m_name; }

private:
    const QString m_bcp47tag;
    const QString m_name;
};


/// An utility class to contain theme informations
class Theme : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString name MEMBER m_name CONSTANT)
    Q_PROPERTY(QString author MEMBER m_author CONSTANT)
    Q_PROPERTY(QString version MEMBER m_version CONSTANT)
    Q_PROPERTY(QString summary MEMBER m_summary CONSTANT)
    Q_PROPERTY(QString description MEMBER m_description CONSTANT)

public:
    explicit Theme(QString root_dir,
                   QString root_qml,
                   QString name,
                   QString author = QString(),
                   QString version = QString(),
                   QString summary = QString(),
                   QString description = QString(),
                   QObject* parent = nullptr);

    const QString& dir() const { return m_root_dir; }
    const QString& name() const { return m_name; }

    int compare(const Theme& other) const;

private:
    const QString m_root_dir;
    const QString m_root_qml;

    const QString m_name;
    const QString m_author;
    const QString m_version;
    const QString m_summary;
    const QString m_description;
};


/// Provides a settings interface for the frontend layer
class Settings : public QObject {
    Q_OBJECT

    // multilanguage support
    Q_PROPERTY(int languageIndex
               READ languageIndex WRITE setLanguageIndex
               NOTIFY languageChanged)
    Q_PROPERTY(QQmlListProperty<ApiParts::Language> allLanguages
               READ getTranslationsProp CONSTANT)

    // theme support
    /*Q_PROPERTY(int themeIndex
               READ themeIndex WRITE setThemeIndex
               NOTIFY themeChanged)
    Q_PROPERTY(QStringList allThemes MEMBER m_theme_qmls CONSTANT)*/

public:
    explicit Settings(QObject* parent = nullptr);

    // multilanguage support
    int languageIndex() const { return m_language_idx; }
    void setLanguageIndex(int idx);
    QQmlListProperty<ApiParts::Language> getTranslationsProp();

    // theme support
    /*int themeIndex() const { return m_theme_idx; }
    void setThemeIndex(int idx);*/

signals:
    void languageChanged();
    //void themeChanged();

private:
    // multilanguage support
    void initLanguages();
    void loadLanguage(const QString& bcp47tag);

    QTranslator m_translator;
    QList<Language*> m_translations;
    int m_language_idx;

    // theme support
    void initThemes();

    QVector<Theme*> m_themes;
    int m_theme_idx;

    // internal
    void callScripts() const;
};

} // namespace ApiParts
