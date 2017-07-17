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

Settings::Settings(QObject* parent)
    : QObject(parent)
    , m_translator(this)
    , m_language_idx(0)
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
    static const auto filters = QDir::Dirs | QDir::Readable | QDir::NoDotAndDotDot;
    static const auto flags = QDirIterator::Subdirectories | QDirIterator::FollowSymlinks;

    QStringList found_theme_paths;

    QStringList search_paths;
    search_paths << QCoreApplication::applicationDirPath();
    search_paths << QStandardPaths::standardLocations(QStandardPaths::AppConfigLocation);
    search_paths << QStandardPaths::standardLocations(QStandardPaths::AppDataLocation);

    for (auto& path : search_paths) {
        path += "/themes/";
        // do not add the organization name to the search path
        path.replace("/pegasus-frontend/pegasus-frontend/", "/pegasus-frontend/");

        QStringList local_themes;

        QDirIterator themedir(path, filters, flags);
        while (themedir.hasNext()) {
            const auto basedir = themedir.next();
            const auto metaini_path = basedir + "metadata.ini";
            const auto mainqml_path = basedir + "main.qml";

            if (!validFile(metaini_path)) {
                qWarning().noquote()
                    << tr("Warning: no `metadata.ini` file found in `%1`, theme skipped")
                       .arg(basedir);
                continue;
            }
            if (!validFile(mainqml_path)) {
                qWarning().noquote()
                    << tr("Warning: no `main.qml` file found in `%1`, theme skipped")
                       .arg(basedir);
                continue;
            }

            local_themes.append(basedir);
        }

        local_themes.sort();
        found_theme_paths.append(local_themes);
    }

    qInfo() << search_paths;
}

void Settings::callScripts() const
{
    using ScriptEvent = ScriptRunner::EventType;

    ScriptRunner::findAndRunScripts(ScriptEvent::CONFIG_CHANGED);
    ScriptRunner::findAndRunScripts(ScriptEvent::SETTINGS_CHANGED);
}

} // namespace ApiParts
