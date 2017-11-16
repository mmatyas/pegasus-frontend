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

#include "Theme.h"

#include <QObject>
#include <QQmlListProperty>
#include <QVector>


namespace Types {

class ThemeList : public QObject {
    Q_OBJECT

    Q_PROPERTY(Types::Theme* current
               READ current
               NOTIFY themeChanged)
    Q_PROPERTY(int index
               READ index
               WRITE setIndex
               NOTIFY themeChanged)
    Q_PROPERTY(int count
               READ count CONSTANT)
    Q_PROPERTY(QQmlListProperty<Types::Theme> model
               READ getListProp CONSTANT)

public:
    explicit ThemeList(QObject* parent = nullptr);

    Theme* current() const;
    int index() const { return m_theme_idx; }
    void setIndex(int idx);
    int count() const { return m_themes.count(); }
    QQmlListProperty<Theme> getListProp();

signals:
    void themeChanged();

private:
    QVector<Theme*> m_themes;
    int m_theme_idx;

    void selectPreferredTheme();
    void printChangeMsg() const;
    int indexOfTheme(const QString&) const;
};

} // namespace Types
