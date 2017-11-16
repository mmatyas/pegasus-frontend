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

#include "Locale.h"

#include <QObject>
#include <QQmlListProperty>
#include <QTranslator>
#include <QVector>


namespace Types {

class LocaleList : public QObject {
    Q_OBJECT

    Q_PROPERTY(Types::Locale* current
               READ current
               NOTIFY localeChanged)
    Q_PROPERTY(int index
               READ index
               WRITE setIndex
               NOTIFY localeChanged)
    Q_PROPERTY(int count
               READ count CONSTANT)
    Q_PROPERTY(QQmlListProperty<Types::Locale> model
               READ getListProp CONSTANT)

public:
    explicit LocaleList(QObject* parent = nullptr);

    Locale* current() const;
    int index() const { return m_locale_idx; }
    void setIndex(int idx);
    int count() const { return m_locales.count(); }
    QQmlListProperty<Locale> getListProp();

signals:
    void localeChanged();

private:
    QVector<Locale*> m_locales;
    int m_locale_idx;

    QTranslator m_translator;

    void selectPreferredLocale();
    void loadSelectedLocale();
    int indexOfLocale(const QString& tag) const;
};

} // namespace Types
