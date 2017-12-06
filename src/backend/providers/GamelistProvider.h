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

#include <QHash>
#include <QVector>
#include <QString>
#include <QObject>

namespace Types { class Game; }
namespace Types { class Collection; }


namespace providers {

class GamelistProvider : public QObject {
    Q_OBJECT

public:
    explicit GamelistProvider(QObject* parent = nullptr);
    virtual ~GamelistProvider();

    virtual void find(QHash<QString, Types::Game*>& games,
                      QHash<QString, Types::Collection*>& collections,
                      QVector<QString>& metadata_dirs) = 0;

    // disable copy
    GamelistProvider(const GamelistProvider&) = delete;
    GamelistProvider& operator=(const GamelistProvider&) = delete;

signals:
    void gameCountChanged(int);
};

} // namespace providers
