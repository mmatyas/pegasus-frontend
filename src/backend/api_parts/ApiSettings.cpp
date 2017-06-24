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


static const char SETTINGSKEY_LOCALE[] = "locale";

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

} // namespace ApiParts
