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
#include "LocaleUtils.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QSettings>


namespace {

const QString SETTINGSKEY_LOCALE(QStringLiteral("locale"));

} // namespace


namespace Types {

LocaleList::LocaleList(QObject* parent)
    : QObject(parent)
    , m_locale_idx(-1)
    , m_translator(this)
{
    findAvailableLocales();
    selectPreferredLocale();
    loadSelectedLocale();

    qApp->installTranslator(&m_translator);
}

void LocaleList::findAvailableLocales()
{
    constexpr int QM_PREFIX_LEN = 8; // length of "pegasus_"
    constexpr int QM_SUFFIX_LEN = 3; // length of ".qm"

    // find the available languages
    QStringList qm_files = QDir(QStringLiteral(":/lang")).entryList(QStringList(QStringLiteral("*.qm")));
    qm_files.append(QStringLiteral("pegasus_en.qm")); // default placeholder
    qm_files.sort();

    for (const QString& filename : qAsConst(qm_files)) {
        const int locale_tag_len = filename.length() - QM_PREFIX_LEN - QM_SUFFIX_LEN;
        Q_ASSERT(locale_tag_len > 0);

        const QString locale_tag = filename.mid(QM_PREFIX_LEN, locale_tag_len);
        m_locales.append(new Types::Locale(locale_tag, this));

        qInfo().noquote() << tr_log("Found locale '%1' (`%2`)")
                             .arg(m_locales.last()->name(), m_locales.last()->tag());
    }
}

void LocaleList::selectPreferredLocale()
{
    // A. Try to use the saved config value
    const QString requested_tag = QSettings().value(SETTINGSKEY_LOCALE).toString();
    if (!requested_tag.isEmpty())
        m_locale_idx = indexOfLocale(requested_tag);

    // B. Try to use the system default language
    if (m_locale_idx < 0)
        m_locale_idx = indexOfLocale(QLocale().bcp47Name());

    // C. Fall back to the default
    if (m_locale_idx < 0)
        m_locale_idx = indexOfLocale(QStringLiteral("en"));


    Q_ASSERT(m_locale_idx >= 0 && m_locale_idx < m_locales.length());
}

void LocaleList::loadSelectedLocale()
{
    m_translator.load(QStringLiteral("pegasus_") + current()->tag(),
                      QStringLiteral(":/lang"),
                      QStringLiteral("-"));
    qInfo().noquote() << tr_log("Locale set to '%1' (`%2`)")
                         .arg(current()->name(), current()->tag());
}

int LocaleList::indexOfLocale(const QString& tag) const
{
    for (int idx = 0; idx < m_locales.count(); idx++) {
        if (m_locales.at(idx)->tag() == tag)
            return idx;
    }

    return -1;
}

Locale* LocaleList::current() const
{
    Q_ASSERT(0 <= index() && index() < m_locales.length());
    return m_locales.at(index());
}

void LocaleList::setIndex(int idx)
{
    // verify
    if (idx == m_locale_idx)
        return;

    const bool valid_idx = (0 <= idx && idx < m_locales.length());
    if (!valid_idx) {
        qWarning() << tr_log("Invalid locale index #%1").arg(idx);
        return;
    }

    // load
    m_locale_idx = idx;
    loadSelectedLocale();
    emit localeChanged();

    // remember
    QSettings().setValue(SETTINGSKEY_LOCALE, current()->tag());
}

QQmlListProperty<Locale> LocaleList::getListProp()
{
    static constexpr auto count = &listproperty_count<Locale>;
    static constexpr auto at = &listproperty_at<Locale>;

    return {this, &m_locales, count, at};
}

} // namespace Types
