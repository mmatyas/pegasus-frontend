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

#include "PropertyListMacro.h"
#include "ScriptRunner.h"
#include "model_providers/AppFiles.h"

#include <QCoreApplication>
#include <QDebug>
#include <QSettings>


namespace {

const QLatin1String SETTINGSKEY_LOCALE("locale");
const QLatin1String SETTINGSKEY_THEME("theme");

void callScripts()
{
    using ScriptEvent = ScriptRunner::EventType;

    ScriptRunner::findAndRunScripts(ScriptEvent::CONFIG_CHANGED);
    ScriptRunner::findAndRunScripts(ScriptEvent::SETTINGS_CHANGED);
}

} // namespace


namespace ApiParts {

//
// Locales
//

LocaleSettings::LocaleSettings(QObject* parent)
    : QObject(parent)
    , m_locales(model_providers::AppFiles::findAvailableLocales())
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
    using model_providers::AppFiles;


    // A. Try to use the saved config value
    const QString requested_tag = QSettings().value(SETTINGSKEY_LOCALE).toString();
    if (!requested_tag.isEmpty())
        m_locale_idx = indexOfLocale(requested_tag);

    // B. Try to use the system default language
    if (m_locale_idx < 0)
        m_locale_idx = indexOfLocale(QLocale().bcp47Name());

    // C. Fall back to the default
    if (m_locale_idx < 0)
        m_locale_idx = indexOfLocale(AppFiles::DEFAULT_LOCALE_TAG);


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
    static const auto count = &listproperty_count<Model::Locale>;
    static const auto at = &listproperty_at<Model::Locale>;

    return {this, &m_locales, count, at};
}

//
// Themes
//

ThemeSettings::ThemeSettings(QObject* parent)
    : QObject(parent)
    , m_themes(model_providers::AppFiles::findAvailableThemes())
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
    static const auto count = &listproperty_count<Model::Theme>;
    static const auto at = &listproperty_at<Model::Theme>;

    return {this, &m_themes, count, at};
}

//
// Settings
//

Settings::Settings(QObject* parent)
    : QObject(parent)
    , m_locales(this)
    , m_themes(this)
{
    m_fullscreen = QSettings().value("fullscreen", true).toBool();
}

void Settings::setFullscreen(bool new_val)
{
    if (new_val != m_fullscreen) {
        m_fullscreen = new_val;
        QSettings().setValue("fullscreen", m_fullscreen);

        emit fullscreenChanged();
    }
}

} // namespace ApiParts
