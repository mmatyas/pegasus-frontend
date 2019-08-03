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

#include "modeldata/GameData.h"
#include "modeldata/CollectionData.h"
#include "utils/HashMap.h"
#include "utils/MoveOnly.h"

#include <QString>
#include <QObject>
#include <vector>

namespace model { class Collection; }
namespace model { class Game; }
namespace model { class GameFile; }


namespace providers {

struct SearchContext {
    HashMap<size_t, modeldata::Game> games;
    HashMap<QString, modeldata::Collection> collections;
    HashMap<QString, std::vector<size_t>> collection_childs;
    HashMap<QString, size_t> path_to_gameid;
    std::vector<QString> game_root_dirs;

    SearchContext() = default;
    MOVE_ONLY(SearchContext)
};


static constexpr uint8_t
    INTERNAL = (1 << 0),
    PROVIDES_GAMES = (1 << 1),
    PROVIDES_ASSETS = (1 << 2),
    PROVIDES_DYNDATA = (1 << 3);


class Provider : public QObject {
    Q_OBJECT

public:
    explicit Provider(QLatin1String codename, QString name, uint8_t flags, QObject* parent = nullptr);
    virtual ~Provider();

    bool enabled() const { return m_enabled; }
    void setEnabled(bool);

    virtual void load() {}
    virtual void unload() {}

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

    // common
    const QLatin1String& codename() const { return m_codename; }
    const QString& name() const { return m_provider_name; }
    uint8_t flags() const { return m_provider_flags; }

    void setOption(const QString&, QString);
    void setOption(const QString&, std::vector<QString>);
    const HashMap<QString, std::vector<QString>>& options() const { return m_options; }

signals:
    void gameCountChanged(int);

private:
    const QLatin1String m_codename;
    const QString m_provider_name;
    const uint8_t m_provider_flags;

    bool m_enabled;
    HashMap<QString, std::vector<QString>> m_options;
};

} // namespace providers
