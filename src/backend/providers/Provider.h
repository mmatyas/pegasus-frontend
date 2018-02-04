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
#include <QString>
#include <QObject>

namespace Types { class Game; }
namespace Types { class Collection; }


namespace providers {

class Provider : public QObject {
    Q_OBJECT

public:
    explicit Provider(QObject* parent = nullptr);
    virtual ~Provider();

    /// Find all games and collections.
    virtual void find(QHash<QString, Types::Game*>& games,
                      QHash<QString, Types::Collection*>& collections) = 0;

    /// Enhance the previously found games and collections with metadata and assets.
    virtual void enhance(const QHash<QString, Types::Game*>& games,
                         const QHash<QString, Types::Collection*>& collections) = 0;

signals:
    void gameCountChanged(int);
    void assetDirFound(QString);
};

} // namespace providers
