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


#include "LocaleList.h"

#include "ListPropertyFn.h"
#include "model_providers/AppFiles.h"

#include <QCoreApplication>
#include <QDebug>
#include <QSettings>


namespace {

const QLatin1String SETTINGSKEY_LOCALE("locale");

} // namespace


namespace Types {

LocaleList::LocaleList(QObject* parent)
    : QObject(parent)
    , m_locales(model_providers::AppFiles::findAvailableLocales())
    , m_locale_idx(-1)
    , m_translator(this)
{
    for (Locale* locale : qAsConst(m_locales)) {
        locale->setParent(this);
        qInfo().noquote() << tr("Found locale '%1' (`%2`)").arg(locale->name(), locale->tag());
    }

    selectPreferredLocale();
    loadSelectedLocale();

    qApp->installTranslator(&m_translator);
}

Locale* LocaleList::current() const
{
    Q_ASSERT(m_locale_idx >= 0 && m_locales.length());
    return m_locales.at(index());
}

int LocaleList::indexOfLocale(const QString& tag) const
{
    for (int idx = 0; idx < m_locales.count(); idx++) {
        if (m_locales.at(idx)->tag() == tag)
            return idx;
    }

    return -1;
}

void LocaleList::selectPreferredLocale()
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

void LocaleList::setIndex(int idx)
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
    emit localeChanged();

    // remember
    QSettings().setValue(SETTINGSKEY_LOCALE, current()->tag());
}

void LocaleList::loadSelectedLocale()
{
    m_translator.load("pegasus_" + current()->tag(), ":/lang", "-");

    qInfo().noquote() << QObject::tr("Locale set to '%1' (`%2`)")
                         .arg(current()->name(), current()->tag());
}

QQmlListProperty<Locale> LocaleList::getListProp()
{
    static const auto count = &listproperty_count<Locale>;
    static const auto at = &listproperty_at<Locale>;

    return {this, &m_locales, count, at};
}

} // namespace Types
