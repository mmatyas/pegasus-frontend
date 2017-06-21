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

Settings::Settings(QObject* parent)
    : QObject(parent)
    , m_translator(this)
{
    loadLanguage(QLocale().bcp47Name());
    qApp->installTranslator(&m_translator);
}

void Settings::setLanguage(const QString& bcp47tag)
{
    if (bcp47tag != m_language) {
        m_language = bcp47tag;
        loadLanguage(bcp47tag);
        emit languageChanged(); // TODO: check load results
    }
}

void Settings::loadLanguage(const QString& bcp47tag)
{
    m_translator.load("pegasus_" + bcp47tag, ":/lang", "-");
}

} // namespace ApiParts
