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

#include <QtGui>


namespace ApiParts {

Language::Language(const QString bcp47tag, const QString name, QObject* parent)
    : QObject(parent)
    , m_bcp47tag(bcp47tag)
    , m_name(name)
{}

Settings::Settings(QObject* parent)
    : QObject(parent)
    , m_translator(this)
    , m_language_idx(0)
{
    m_translations.append(new Language("en", "English", this));
    m_language_idx = m_translations.length() - 1; // default language is english
    m_translations.append(new Language("hu", "Magyar", this));
    m_translations.append(new Language("hu-Hung", u8"\u202E𐳢𐳛𐳮𐳁𐳤", this));

    // TODO: check if there's a stored value in the config file

    // try to find the system default language
    const QString system_lang_tag = QLocale().bcp47Name();
    for (int i = 0; i < m_translations.length(); i++) {
        if (m_translations[i]->tag() == system_lang_tag) {
            qDebug().noquote() << tr("Found translation for `%1`").arg(system_lang_tag);
            m_language_idx = i;
            break;
        }
    }

    Q_ASSERT(m_language_idx >= 0 && m_language_idx < m_translations.length());
    loadLanguage(m_translations.at(m_language_idx)->tag());
    qApp->installTranslator(&m_translator);
}

void Settings::setLanguageIndex(int idx)
{
    if (idx == m_language_idx)
        return;

    const bool valid_idx = (0 <= idx && idx < m_translations.length());
    if (!valid_idx) {
        qWarning() << tr("Invalid language index #%1").arg(idx);
        return;
    }

    m_language_idx = idx;
    loadLanguage(m_translations.at(idx)->tag());
    emit languageChanged();

    // TODO: save to config
}

void Settings::loadLanguage(const QString& bcp47tag)
{
    m_translator.load("pegasus_" + bcp47tag, ":/lang", "-");
}

QQmlListProperty<ApiParts::Language> Settings::getTranslationsProp()
{
    return QQmlListProperty<ApiParts::Language>(this, m_translations);
}

} // namespace ApiParts
