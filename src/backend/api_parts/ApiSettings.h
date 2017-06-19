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


#pragma once

#include <QObject>
#include <QTranslator>


namespace ApiParts {

class Settings : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString language
               READ language WRITE setLanguage
               NOTIFY languageChanged)

public:
    explicit Settings(QObject* parent = nullptr);

    QString language() const { return m_language; }
    void setLanguage(const QString& bcp47tag);

signals:
    void languageChanged();

private:
    void loadLanguage(const QString& bcp47tag);

    QString m_language;
    QTranslator m_translator;
};

} // namespace ApiParts
