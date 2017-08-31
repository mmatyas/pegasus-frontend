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


namespace {

const QLatin1String SETTINGSKEY_LOCALE("locale");
const QLatin1String SETTINGSKEY_THEME("theme");
const QLatin1String DEFAULT_LOCALE_TAG("en-DK");

void callScripts()
{
    using ScriptEvent = ScriptRunner::EventType;

    ScriptRunner::findAndRunScripts(ScriptEvent::CONFIG_CHANGED);
    ScriptRunner::findAndRunScripts(ScriptEvent::SETTINGS_CHANGED);
}

QList<Model::Locale*> findAvailableLocales()
{
    const int QM_PREFIX_LEN = 8; // length of "pegasus_"
    const int QM_SUFFIX_LEN = 3; // length of ".qm"
    const QString DEFAULT_FILENAME("pegasus_" + DEFAULT_LOCALE_TAG + ".qm");

    // find the available languages
    QStringList qm_files = QDir(":/lang").entryList(QStringList("*.qm"));
    qm_files.append(DEFAULT_FILENAME); // default placeholder
    qm_files.sort();

    QList<Model::Locale*> output;
    for (const QString& filename : qAsConst(qm_files)) {
        const int locale_tag_len = filename.length() - QM_PREFIX_LEN - QM_SUFFIX_LEN;
        Q_ASSERT(locale_tag_len > 0);

        const QString locale_tag = filename.mid(QM_PREFIX_LEN, locale_tag_len);
        output.append(new Model::Locale(locale_tag));
    }
    return output;
}

QStringList themeDirectories()
{
    QStringList theme_dirs;
    theme_dirs << ":";
    theme_dirs << QCoreApplication::applicationDirPath();
#ifdef INSTALL_DATADIR
    if (validPath(INSTALL_DATADIR))
        theme_dirs << QString(INSTALL_DATADIR);
#endif
    theme_dirs << QStandardPaths::standardLocations(QStandardPaths::AppConfigLocation);
    theme_dirs << QStandardPaths::standardLocations(QStandardPaths::AppDataLocation);

    for (QString& path : theme_dirs) {
        path += "/themes/";
        // do not add the organization name to the search path
        path.replace(QLatin1String("/pegasus-frontend/pegasus-frontend/"),
                     QLatin1String("/pegasus-frontend/"));
    }

    return theme_dirs;
}

// because the theme files may be embedded,
// we can't use platform-specific shortcuts
bool fileExists(const QString& path)
{
    QFileInfo file(path);
    return file.exists() && file.isFile();
};

QList<Model::Theme*> findAvailableThemes()
{
    const auto filters = QDir::Dirs | QDir::Readable | QDir::NoDotAndDotDot;
    const auto flags = QDirIterator::FollowSymlinks;

    const QString ini_filename("theme.ini");
    const QString qml_filename("theme.qml");
    const QString warn_missingfile = QObject::tr("Warning: no `%1` file found in `%2`, theme skipped");
    const QString warn_missingentry = QObject::tr("Warning: there is no `%1` entry in `%2`, theme skipped");

    const QString INIKEY_NAME("name");
    const QString INIKEY_AUTHOR("author");
    const QString INIKEY_VERSION("version");
    const QString INIKEY_SUMMARY("summary");
    const QString INIKEY_DESC("description");

    QList<Model::Theme*> output;

    QStringList search_paths = themeDirectories();
    for (auto& path : search_paths) {
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

            output.append(new Model::Theme(
                basedir, qml_path,
                metadata.value(INIKEY_NAME).toString(),
                metadata.value(INIKEY_AUTHOR).toString(),
                metadata.value(INIKEY_VERSION).toString(),
                metadata.value(INIKEY_SUMMARY).toString(),
                metadata.value(INIKEY_DESC).toString()
            ));
        }
    }

    std::sort(output.begin(), output.end(),
        [](const Model::Theme* a, const Model::Theme* b) {
            return a->compare(*b) < 0;
        }
    );

    return output;
}

} // namespace


namespace ApiParts {

//
// Locales
//

LocaleSettings::LocaleSettings(QObject* parent)
    : QObject(parent)
    , m_locales(findAvailableLocales())
    , m_locale_idx(-1)
    , m_translator(this)
{
    for (Model::Locale* locale : qAsConst(m_locales)) {
        locale->setParent(this);
        qInfo().noquote() << tr("Found locale '%1' (`%2`)").arg(locale->name(), locale->tag());
    }

    selectPreferredLocale();
    loadSelectedLocale();

    qApp->installTranslator(&m_translator);
}

int LocaleSettings::indexOfLocale(const QString& tag) const
{
    for (int idx = 0; idx < m_locales.count(); idx++) {
        if (m_locales.at(idx)->tag() == tag)
            return idx;
    }

    return -1;
}

void LocaleSettings::selectPreferredLocale()
{
    // this method should be called after all translations have been found
    Q_ASSERT(!m_locales.isEmpty());


    // A. Try to use the saved config value
    const QString requested_tag = QSettings().value(SETTINGSKEY_LOCALE).toString();
    if (!requested_tag.isEmpty())
        m_locale_idx = indexOfLocale(requested_tag);

    // B. Try to use the system default language
    if (m_locale_idx < 0)
        m_locale_idx = indexOfLocale(QLocale().bcp47Name());

    // C. Fall back to the default
    if (m_locale_idx < 0)
        m_locale_idx = indexOfLocale(DEFAULT_LOCALE_TAG);


    Q_ASSERT(m_locale_idx >= 0 && m_locale_idx < m_locales.length());
}

void LocaleSettings::setIndex(int idx)
{
    // verify
    if (idx == m_locale_idx)
        return;

    const bool valid_idx = (0 <= idx && idx < m_locales.length());
    if (!valid_idx) {
        qWarning() << QObject::tr("Invalid locale index #%1").arg(idx);
        return;
    }

    // load
    m_locale_idx = idx;
    loadSelectedLocale();

    // remember
    QSettings().setValue(SETTINGSKEY_LOCALE, current()->tag());

    callScripts();
    emit localeChanged();
}

void LocaleSettings::loadSelectedLocale()
{
    m_translator.load("pegasus_" + current()->tag(), ":/lang", "-");

    qInfo().noquote() << QObject::tr("Locale set to '%1' (`%2`)")
                         .arg(current()->name(), current()->tag());
}

QQmlListProperty<Model::Locale> LocaleSettings::getListProp()
{
    return QQmlListProperty<Model::Locale>(this, m_locales);
}

//
// Themes
//

ThemeSettings::ThemeSettings(QObject* parent)
    : QObject(parent)
    , m_themes(findAvailableThemes())
    , m_theme_idx(-1)
{
    for (Model::Theme* theme : qAsConst(m_themes)) {
        theme->setParent(this);
        qInfo().noquote() << tr("Found theme '%1' (`%2`)").arg(theme->name(), theme->dir());
    }

    selectPreferredTheme();
    printChangeMsg();
}

int ThemeSettings::indexOfTheme(const QString& dir_path) const
{
    for (int idx = 0; idx < m_themes.count(); idx++) {
        if (m_themes.at(idx)->dir() == dir_path)
            return idx;
    }

    return -1;
}

void ThemeSettings::selectPreferredTheme()
{
    // this method should be called after all themes have been found
    Q_ASSERT(!m_themes.isEmpty());


    // A. Try to use the saved config value
    const QString requested_theme = QSettings().value(SETTINGSKEY_THEME).toString();
    if (!requested_theme.isEmpty())
        m_theme_idx = indexOfTheme(requested_theme);

    // B. Fall back to the built-in theme
    //    Either the config value is invalid, or has missing files,
    //    thus not present in `m_themes`.
    if (m_theme_idx < 0)
        m_theme_idx = indexOfTheme(":/themes/pegasus-grid/");


    Q_ASSERT(m_theme_idx >= 0 && m_theme_idx < m_themes.length());
}

void ThemeSettings::setIndex(int idx)
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
    printChangeMsg();

    // remember
    QSettings().setValue(SETTINGSKEY_THEME, m_themes.at(idx)->dir());

    callScripts();
    emit themeChanged();
}

void ThemeSettings::printChangeMsg() const
{
    qInfo().noquote() << QObject::tr("Theme set to '%1' (%2)")
                         .arg(current()->name(), current()->dir());
}

QQmlListProperty<Model::Theme> ThemeSettings::getListProp()
{
    return QQmlListProperty<Model::Theme>(this, m_themes);
}

//
// Settings
//

Settings::Settings(QObject* parent)
    : QObject(parent)
    , m_locales(this)
    , m_themes(this)
{
}

} // namespace ApiParts
