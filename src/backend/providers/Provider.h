// Pegasus Frontend
// Copyright (C) 2017-2019  Mátyás Mustoha
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

#include "modeldata/gaming/GameData.h"
#include "modeldata/gaming/CollectionData.h"
#include "utils/FwdDeclModel.h"
#include "utils/HashMap.h"

#include <QString>
#include <QObject>
#include <vector>


namespace providers {

struct SearchContext {
    HashMap<size_t, modeldata::Game> games;
    HashMap<QString, modeldata::Collection> collections;
    HashMap<QString, std::vector<size_t>> collection_childs;
    HashMap<QString, size_t> path_to_gameid;
};

class Provider : public QObject {
    Q_OBJECT

public:
    explicit Provider(QObject* parent = nullptr);
    virtual ~Provider();

    /// Initialization first stage:
    /// Find all games and collections.
    virtual void findLists(SearchContext&) {}

    /// Initialization second stage:
    /// Enhance the previously found games and collections with metadata and assets.
    virtual void findStaticData(SearchContext&) {}

    /// Initialization third stage:
    /// Find data that may change during the runtime for all games
    virtual void findDynamicData(const QVector<model::Collection*>&,
                                 const QVector<model::Game*>&,
                                 const HashMap<QString, model::GameFile*>&) {}


    // events
    virtual void onGameFavoriteChanged(const QVector<model::Game*>&) {}
    virtual void onGameLaunched(model::GameFile* const) {}
    virtual void onGameFinished(model::GameFile* const) {}

signals:
    void gameCountChanged(int);
};

} // namespace providers
