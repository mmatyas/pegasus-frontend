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

#include "utils/FwdDeclModel.h"
#include "utils/FwdDeclModelData.h"
#include "utils/HashMap.h"

#include <QString>
#include <QObject>
#include <QVector>
#include <vector>


namespace providers {

class Provider : public QObject {
    Q_OBJECT

public:
    explicit Provider(QObject* parent = nullptr);
    virtual ~Provider();

    /// Initialization first stage:
    /// Find all games and collections.
    virtual void findLists(HashMap<QString, modeldata::Game>&,
                           HashMap<QString, modeldata::Collection>&,
                           HashMap<QString, std::vector<QString>>&)
    {}

    /// Initialization second stage:
    /// Enhance the previously found games and collections with metadata and assets.
    virtual void findStaticData(HashMap<QString, modeldata::Game>&,
                                const HashMap<QString, modeldata::Collection>&,
                                const HashMap<QString, std::vector<QString>>&)
    {}

    /// Initialization third stage:
    /// Find data that may change during the runtime for all games
    virtual void findDynamicData(const QVector<model::Game*>&,
                                 const QVector<model::Collection*>&,
                                 const HashMap<QString, model::Game*>&)
    {}


    // events
    virtual void onGameFavoriteChanged(const QVector<model::Game*>&) {}
    virtual void onGameLaunched(model::Collection* const, model::Game* const) {}
    virtual void onGameFinished(model::Collection* const, model::Game* const) {}

signals:
    void gameCountChanged(int);
};

} // namespace providers
