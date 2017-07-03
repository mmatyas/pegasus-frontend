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
#include <QQmlListProperty>
#include <QTranslator>


namespace ApiParts {

class Language : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString tag MEMBER m_bcp47tag CONSTANT)
    Q_PROPERTY(QString name MEMBER m_name CONSTANT)

public:
    explicit Language(QString bcp47tag, QString name,
                      QObject* parent = nullptr);

    const QString tag() const { return m_bcp47tag; }
    const QString name() const { return m_name; }

private:
    const QString m_bcp47tag;
    const QString m_name;
};


class Settings : public QObject {
    Q_OBJECT

    Q_PROPERTY(int languageIndex
               READ languageIndex WRITE setLanguageIndex
               NOTIFY languageChanged)
    Q_PROPERTY(QQmlListProperty<ApiParts::Language> allLanguages
               READ getTranslationsProp CONSTANT)

public:
    explicit Settings(QObject* parent = nullptr);

    int languageIndex() const { return m_language_idx; }
    void setLanguageIndex(int idx);

    QQmlListProperty<ApiParts::Language> getTranslationsProp();

signals:
    void languageChanged();

private:
    void loadLanguage(const QString& bcp47tag);

    QTranslator m_translator;
    int m_language_idx;
    QList<Language*> m_translations;
};

} // namespace ApiParts
