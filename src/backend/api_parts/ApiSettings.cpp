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


#include "ApiSettings.h"

#include "ScriptRunner.h"
#include "Utils.h"

#include <QtGui>


static const char SETTINGSKEY_LOCALE[] = "locale";
static const char SETTINGSKEY_THEME[] = "theme";

namespace ApiParts {

Language::Language(QString bcp47tag, QString name, QObject* parent)
    : QObject(parent)
    , m_bcp47tag(bcp47tag)
    , m_name(name)
{}

Theme::Theme(QString root_dir, QString root_qml,
             QString name, QString author, QString version,
             QString summary, QString description,
             QObject* parent)
    : QObject(parent)
    , m_root_dir(root_dir)
    , m_root_qml(root_qml.startsWith(":") ? "qrc" + root_qml : "file:" + root_qml)
    , m_name(name)
    , m_author(author)
    , m_version(version)
    , m_summary(summary)
    , m_description(description)
{}

int Theme::compare(const Theme& other) const
{
    return QString::localeAwareCompare(m_name, other.m_name);
}

Settings::Settings(QObject* parent)
    : QObject(parent)
    , m_translator(this)
    , m_language_idx(-1)
    , m_theme_idx(-1)
{
    initLanguages();
    initThemes();
}

void Settings::initLanguages()
{
    m_translations.append(new Language("en", "English", this));
    m_language_idx = m_translations.length() - 1; // fallback language is english
    m_translations.append(new Language("hu", "Magyar", this));
    m_translations.append(new Language("hu-Hung", u8"\u202E𐳢𐳛𐳮𐳁𐳤", this));

    // if there is a saved language setting, use that
    // if not, use the system language
    const QString requested_tag = [](){
        QVariant entry = QSettings().value(SETTINGSKEY_LOCALE);
        return entry.isNull() ? QLocale().bcp47Name() : entry.toString();
    }();

    // try to find the saved/system language
    // or fall back to english
    for (int i = 0; i < m_translations.length(); i++) {
        if (m_translations[i]->tag() == requested_tag) {
            qDebug().noquote() << tr("Found translation for `%1`").arg(requested_tag);
            m_language_idx = i;
            break;
        }
    }

    // load
    Q_ASSERT(m_language_idx >= 0 && m_language_idx < m_translations.length());
    loadLanguage(m_translations.at(m_language_idx)->tag());
    qApp->installTranslator(&m_translator);
}

void Settings::setLanguageIndex(int idx)
{
    // verify
    if (idx == m_language_idx)
        return;

    const bool valid_idx = (0 <= idx && idx < m_translations.length());
    if (!valid_idx) {
        qWarning() << tr("Invalid language index #%1").arg(idx);
        return;
    }

    // load
    m_language_idx = idx;
    loadLanguage(m_translations.at(idx)->tag());

    // remember
    QSettings().setValue(SETTINGSKEY_LOCALE, m_translations.at(idx)->tag());

    callScripts();
    emit languageChanged();
}

void Settings::loadLanguage(const QString& bcp47tag)
{
    m_translator.load("pegasus_" + bcp47tag, ":/lang", "-");
}

QQmlListProperty<ApiParts::Language> Settings::getTranslationsProp()
{
    return QQmlListProperty<ApiParts::Language>(this, m_translations);
}

void Settings::initThemes()
{
    const auto filters = QDir::Dirs | QDir::Readable | QDir::NoDotAndDotDot;
    const auto flags = QDirIterator::Subdirectories | QDirIterator::FollowSymlinks;

    const QString ini_filename = "theme.ini";
    const QString qml_filename = "theme.qml";
    const QString warn_missingfile = tr("Warning: no `%1` file found in `%2`, theme skipped");
    const QString warn_missingentry = tr("Warning: there is no `%1` entry in `%2`, theme skipped");

    const QString INIKEY_NAME = "name";
    const QString INIKEY_AUTHOR = "author";
    const QString INIKEY_VERSION = "version";
    const QString INIKEY_SUMMARY = "summary";
    const QString INIKEY_DESC = "description";

    QStringList search_paths;
    search_paths << ":";
    search_paths << QCoreApplication::applicationDirPath();
#ifdef INSTALL_DATADIR
    if (validFile(INSTALL_DATADIR))
        search_paths << QString(INSTALL_DATADIR);
#endif
    search_paths << QStandardPaths::standardLocations(QStandardPaths::AppConfigLocation);
    search_paths << QStandardPaths::standardLocations(QStandardPaths::AppDataLocation);

    // because the theme files can be in a QRC, we can't use the
    // `validFile` function, which uses POSIX `stat` on Unix
    const auto fileExists = [](const QString& path) -> bool {
        QFileInfo file(path);
        return file.exists() && file.isFile();
    };

    for (auto& path : search_paths) {
        path += "/themes/";
        // do not add the organization name to the search path
        path.replace("/pegasus-frontend/pegasus-frontend/", "/pegasus-frontend/");

        QDirIterator themedir(path, filters, flags);

        while (themedir.hasNext()) {
            const auto basedir = themedir.next() + '/';
            const auto ini_path = basedir + ini_filename;
            const auto qml_path = basedir + qml_filename;

            if (!fileExists(ini_path)) {
                qWarning().noquote() << warn_missingfile.arg(ini_filename).arg(basedir);
                continue;
            }
            if (!fileExists(qml_path)) {
                qWarning().noquote() << warn_missingfile.arg(qml_filename).arg(basedir);
                continue;
            }

            const QSettings metadata(ini_path, QSettings::IniFormat);
            if (!metadata.contains(INIKEY_NAME)) {
                qWarning().noquote() << warn_missingentry.arg(INIKEY_NAME).arg(ini_filename);
                continue;
            }

            m_themes.append(new Theme(
                basedir, qml_path,
                metadata.value(INIKEY_NAME).toString(),
                metadata.value(INIKEY_AUTHOR).toString(),
                metadata.value(INIKEY_VERSION).toString(),
                metadata.value(INIKEY_SUMMARY).toString(),
                metadata.value(INIKEY_DESC).toString()
            ));
        }
    }

    std::sort(m_themes.begin(), m_themes.end(),
        [](const Theme* a, const Theme* b) {
            return a->compare(*b) < 0;
        }
    );

    for (const auto& theme : m_themes) {
        qInfo().noquote() << tr("Found theme: %1 (`%2`)").arg(theme->name()).arg(theme->dir());
    }


    // if there is a saved theme setting, use that
    // if not, use the default grid theme
    const QString requested_theme = [](){
        QVariant entry = QSettings().value(SETTINGSKEY_THEME);
        return entry.isNull() ? ":/themes/pegasus-grid/" : entry.toString();
    }();

    // find the selected theme
    // this must always succeed; there's a fallback built-in theme after all
    for (int i = 0; i < m_themes.length(); i++) {
        if (m_themes[i]->dir() == requested_theme) {
            qInfo().noquote() << tr("Theme set to '%1'").arg(m_themes[i]->name());
            m_theme_idx = i;
            break;
        }
    }

    Q_ASSERT(m_theme_idx >= 0 && m_theme_idx < m_themes.length());
}

void Settings::setThemeIndex(int idx)
{
    // verify
    if (idx == m_theme_idx)
        return;

    const bool valid_idx = (0 <= idx && idx < m_themes.length());
    if (!valid_idx) {
        qWarning() << tr("Invalid theme index #%1").arg(idx);
        return;
    }

    // set
    m_theme_idx = idx;

    // remember
    QSettings().setValue(SETTINGSKEY_THEME, m_themes.at(idx)->dir());

    callScripts();
    emit themeChanged();
}

QQmlListProperty<ApiParts::Theme> Settings::getThemesProp()
{
    return QQmlListProperty<ApiParts::Theme>(this, m_themes);
}

void Settings::callScripts() const
{
    using ScriptEvent = ScriptRunner::EventType;

    ScriptRunner::findAndRunScripts(ScriptEvent::CONFIG_CHANGED);
    ScriptRunner::findAndRunScripts(ScriptEvent::SETTINGS_CHANGED);
}

} // namespace ApiParts
