// Pegasus Frontend
// Copyright (C) 2017-2018  Mátyás Mustoha
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


#include "Locales.h"

#include "AppSettings.h"
#include "Log.h"

#include <QCoreApplication>
#include <QDir>
#include <QLocale>


namespace {
std::vector<model::LocaleEntry> find_available_locales()
{
    constexpr int QM_PREFIX_LEN = 8; // length of "pegasus_"
    constexpr int QM_SUFFIX_LEN = 3; // length of ".qm"

    QStringList qm_files = QDir(QStringLiteral(":/i18n")).entryList(QStringList(QStringLiteral("*.qm")));
    qm_files.append(QStringLiteral("pegasus_en.qm"));
    qm_files.sort();

    std::vector<model::LocaleEntry> locales;
    locales.reserve(static_cast<size_t>(qm_files.count()));

    for (const QString& filename : qAsConst(qm_files)) {
        const int locale_tag_len = filename.length() - QM_PREFIX_LEN - QM_SUFFIX_LEN;
        Q_ASSERT(locale_tag_len > 0);

        QString locale_tag = filename.mid(QM_PREFIX_LEN, locale_tag_len);
        locales.emplace_back(std::move(locale_tag));

        Log::info(tr_log("Found locale `%2`").arg(locales.back().bcp47tag));
    }

    return locales;
}
} // namespace


namespace model {

LocaleEntry::LocaleEntry(QString tag)
    : bcp47tag(std::move(tag))
    , name(QLocale(bcp47tag).nativeLanguageName())
{}

Locales::Locales(QObject* parent)
    : QAbstractListModel(parent)
    , m_role_names({
        { Roles::Tag, QByteArrayLiteral("tag") },
        { Roles::Name, QByteArrayLiteral("name") },
    })
    , m_locales(find_available_locales())
{
    select_preferred_locale();
    load_selected_locale();

    qApp->installTranslator(&m_translator);
}

void Locales::select_preferred_locale()
{
    // A. Try to use the saved config value
    if (select_locale(AppSettings::general.locale))
        return;

    // B. Try to use the system default language
    if (select_locale(QLocale().bcp47Name()))
        return;

    // C. Fall back to the default
    if (select_locale(AppSettings::general.DEFAULT_LOCALE))
        return;

    Q_UNREACHABLE();
}

bool Locales::select_locale(const QString& tag)
{
    if (tag.isEmpty())
        return false;

    for (size_t idx = 0; idx < m_locales.size(); idx++) {
        if (m_locales.at(idx).bcp47tag == tag) {
            m_current_idx = idx;
            return true;
        }
    }

    return false;
}

void Locales::load_selected_locale()
{
    const auto& locale = m_locales.at(m_current_idx);

    m_translator.load(QStringLiteral("pegasus_") + locale.bcp47tag,
                      QStringLiteral(":/i18n"),
                      QStringLiteral("-"));
    Log::info(tr_log("Locale set to `%2`").arg(locale.bcp47tag));
}

int Locales::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;

    return static_cast<int>(m_locales.size());
}

QVariant Locales::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || rowCount() <= index.row())
        return {};

    const auto& locale = m_locales.at(static_cast<size_t>(index.row()));
    switch (role) {
        case Roles::Tag:
            return locale.bcp47tag;
        case Roles::Name:
            return locale.name;
        default:
            return {};
    }
}

void Locales::setCurrentIndex(int idx_int)
{
    const auto idx = static_cast<size_t>(idx_int);

    // verify
    if (idx == m_current_idx)
        return;

    if (m_locales.size() <= idx) {
        Log::warning(tr_log("Invalid locale index #%1").arg(idx));
        return;
    }

    // load
    m_current_idx = idx;
    load_selected_locale();
    emit localeChanged();

    // remember
    AppSettings::general.locale = m_locales.at(idx).bcp47tag;
    AppSettings::save_config();
}

} // namespace model
