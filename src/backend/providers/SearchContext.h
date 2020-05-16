// Pegasus Frontend
// Copyright (C) 2017-2020  Mátyás Mustoha
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

#include "utils/HashMap.h"
#include "utils/MoveOnly.h"

#include <QFileInfo>
#include <set>
#include <vector>

namespace model { class Game; }
namespace model { class GameFile; }
namespace model { class Collection; }


namespace providers {
class PendingCollection {
    const QString m_id;
    model::Collection* m_ptr;
    std::set<size_t> m_game_ids;

public:
    PendingCollection(QString, model::Collection*);
    ~PendingCollection();

    const QString& id() const { return m_id; }
    model::Collection& inner() const { return *m_ptr; }
    model::Collection* ptr() const { return m_ptr; }
    const std::set<size_t>& game_ids() const { return m_game_ids; }

    model::Collection* take_ptr();

friend class SearchContext;
};


class PendingGame {
    const size_t m_id;
    model::Game* m_ptr;
    std::set<QString> m_collection_ids;
    std::vector<model::GameFile*> m_files;

public:
    PendingGame(size_t, model::Game*);
    ~PendingGame();

    size_t id() const { return m_id; }
    model::Game& inner() const { return *m_ptr; }
    model::Game* ptr() const { return m_ptr; }
    const std::set<QString>& collection_ids() const { return m_collection_ids; }
    const std::vector<model::GameFile*>& files() const { return m_files; }

    model::Game* take_ptr();

friend class SearchContext;
};


class SearchContext {
    HashMap<size_t, PendingGame> m_games;
    HashMap<QString, PendingCollection> m_collections;
    HashMap<QString, size_t> m_entryid_to_gameid;
    std::set<QString> m_game_root_dirs;

public:
    SearchContext() = default;
    MOVE_ONLY(SearchContext)

    SearchContext& add_game_root_dir(QString);

    PendingCollection& get_or_create_collection(QString);

    PendingGame& create_bare_game_for(QString, PendingCollection* const);
    PendingGame& add_or_create_game_from_entry(QString, PendingCollection&);
    PendingGame& add_or_create_game_from_file(QFileInfo, PendingCollection&);

    SearchContext& create_game_file_for(QFileInfo, PendingGame&);
    SearchContext& create_game_file_with_name_for(QFileInfo, QString, PendingGame&);

    const decltype(m_games)& games() const { return m_games; }
    const decltype(m_collections)& collections() const { return m_collections; }
    const decltype(m_entryid_to_gameid)& entryid_to_gameid() const { return m_entryid_to_gameid; }
    const decltype(m_game_root_dirs)& game_root_dirs() const { return m_game_root_dirs; }

    SearchContext& finalize_lists();
    std::tuple<QVector<model::Collection*>, QVector<model::Game*>> consume();

private:
    PendingGame& create_game_from_file(QFileInfo, PendingCollection&);
    PendingGame& register_game(model::Game* const, PendingCollection* const);

    void remove_invalid_items();
};
} // namespace providers
