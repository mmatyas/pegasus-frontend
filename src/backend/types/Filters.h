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


namespace Types {

/// Stores parameters to filter the list of games
class Filters : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString title MEMBER m_title NOTIFY filtersChanged)
    Q_PROPERTY(int playerCount MEMBER m_player_count NOTIFY filtersChanged)
    Q_PROPERTY(bool favorite MEMBER m_favorite NOTIFY filtersChanged)

public:
    explicit Filters(QObject* parent = nullptr);

    QString m_title;
    bool m_favorite;
    int m_player_count;

signals:
    void filtersChanged();
};

} // namespace Types
