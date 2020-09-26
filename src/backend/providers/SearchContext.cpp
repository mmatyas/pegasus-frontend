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


#include "SearchContext.h"

#include "LocaleUtils.h"
#include "model/gaming/Game.h"
#include "utils/StdHelpers.h"


namespace providers {
PendingGame::PendingGame(size_t id, model::Game* ptr)
    : m_id(id)
    , m_ptr(ptr)
{
    Q_ASSERT(m_ptr);
}

PendingGame::~PendingGame() {
    if (m_ptr)
        delete m_ptr;
}

model::Game* PendingGame::take_ptr() {
    model::Game* ptr = m_ptr;
    m_ptr = nullptr;
    return ptr;
}

PendingCollection::PendingCollection(QString id, model::Collection* ptr)
    : m_id(std::move(id))
    , m_ptr(ptr)
{
    Q_ASSERT(m_ptr);
}

PendingCollection::~PendingCollection() {
    if (m_ptr)
        delete m_ptr;
}

model::Collection* PendingCollection::take_ptr() {
    model::Collection* ptr = m_ptr;
    m_ptr = nullptr;
    return ptr;
}


SearchContext& SearchContext::add_game_root_dir(QString val)
{
    m_game_root_dirs.emplace(std::move(val));
    return *this;
}

PendingGame&
SearchContext::register_game(model::Game* const game_ptr, PendingCollection* const collection)
{
    size_t entry_id = m_games.size();
    auto result = m_games.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(entry_id),
        std::forward_as_tuple(entry_id, game_ptr));

    if (collection) {
        // TODO: make launch parameter setup well defined
        PendingGame& game = result.first->second;
        game.inner()
            .setLaunchCmd(collection->inner().commonLaunchCmd())
            .setLaunchWorkdir(collection->inner().commonLaunchWorkdir())
            .setLaunchCmdBasedir(collection->inner().commonLaunchCmdBasedir());
        collection->m_game_ids.insert(game.id());
        game.m_collection_ids.insert(collection->id());
    }

    return result.first->second;
}

PendingGame&
SearchContext::create_bare_game_for(QString name, PendingCollection* const collection_ptr)
{
    return register_game(new model::Game(name), collection_ptr);
}

PendingGame&
SearchContext::add_or_create_game_from_entry(QString entryid, PendingCollection& collection)
{
    auto slot = m_entryid_to_gameid.find(entryid);
    if (slot != m_entryid_to_gameid.end()) {
        size_t game_id = slot->second;
        collection.m_game_ids.insert(game_id);

        Q_ASSERT(game_id < m_games.size());
        PendingGame& game = m_games.at(game_id);
        game.m_collection_ids.insert(collection.id());

        return game;
    }

    model::Game* game_ptr = new model::Game(model::pretty_filename(entryid));
    PendingGame& game = register_game(game_ptr, &collection);

    auto file = new model::GameFile(QFileInfo(entryid), game.ptr()); // TODO
    game.m_files.emplace_back(file);

    m_entryid_to_gameid.emplace(entryid, game.id());
    return game;
}

PendingGame&
SearchContext::add_or_create_game_from_file(QFileInfo fi, PendingCollection& collection)
{
    QString file_path = fi.canonicalFilePath();
    Q_ASSERT(!file_path.isEmpty());

    auto slot = m_entryid_to_gameid.find(file_path);
    if (slot != m_entryid_to_gameid.end()) {
        size_t game_id = slot->second;
        collection.m_game_ids.insert(game_id);

        Q_ASSERT(game_id < m_games.size());
        PendingGame& game = m_games.at(game_id);
        game.m_collection_ids.insert(collection.id());

        return game;
    }

    return create_game_from_file(std::move(fi), collection);
}

PendingCollection& SearchContext::get_or_create_collection(QString name)
{
    auto found = m_collections.find(name);
    if (found != m_collections.end())
        return found->second;

    auto ptr = new model::Collection(name);
    Q_ASSERT(ptr);

    auto result = m_collections.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(name),
        std::forward_as_tuple(name, ptr));
    return result.first->second;
}

SearchContext& SearchContext::create_game_file_for(QFileInfo fi, PendingGame& game)
{
    QString file_path = fi.canonicalFilePath();
    Q_ASSERT(!file_path.isEmpty());
    Q_ASSERT(!m_entryid_to_gameid.count(file_path));

    m_entryid_to_gameid.emplace(std::move(file_path), game.id());
    auto file = new model::GameFile(std::move(fi), game.ptr());
    game.m_files.emplace_back(file);
    return *this;
}

SearchContext& SearchContext::create_game_file_with_name_for(QFileInfo fi, QString name, PendingGame& game)
{
    QString file_path = fi.canonicalFilePath();
    Q_ASSERT(!file_path.isEmpty());
    Q_ASSERT(!m_entryid_to_gameid.count(file_path));

    m_entryid_to_gameid.emplace(std::move(file_path), game.id());
    auto file = new model::GameFile(std::move(fi), std::move(name), game.ptr());
    game.m_files.emplace_back(file);
    return *this;
}

SearchContext& SearchContext::finalize_lists()
{
    remove_invalid_items();

    for (auto& entry : m_games) {
        std::vector<model::Collection*> collections;
        std::transform(
            entry.second.collection_ids().cbegin(),
            entry.second.collection_ids().cend(),
            std::back_inserter(collections),
            [this](const QString& id) { return m_collections.at(id).ptr(); });
        entry.second.inner().setCollections(std::move(collections));
        entry.second.inner().setFiles(std::move(entry.second.m_files));
    }

    for (auto& entry : m_collections) {
        std::vector<model::Game*> games;
        std::transform(
            entry.second.game_ids().cbegin(),
            entry.second.game_ids().cend(),
            std::back_inserter(games),
            [this](const size_t id) { return m_games.at(id).ptr(); });
        entry.second.inner().setGames(std::move(games));
    }

    return *this;
}

std::tuple<QVector<model::Collection*>, QVector<model::Game*>> SearchContext::consume()
{
    QVector<model::Collection*> out_collections;
    out_collections.reserve(collections().size());
    for (auto& entry : m_collections)
        out_collections.push_back(entry.second.take_ptr());

    QVector<model::Game*> out_games;
    out_games.reserve(games().size());
    for (auto& entry : m_games)
        out_games.push_back(entry.second.take_ptr());

    std::sort(out_collections.begin(), out_collections.end(), model::sort_collections);
    std::sort(out_games.begin(), out_games.end(), model::sort_games);

    return std::make_tuple(std::move(out_collections), std::move(out_games));
}

PendingGame&
SearchContext::create_game_from_file(QFileInfo fi, PendingCollection& collection)
{
    model::Game* game = new model::Game(model::pretty_filename(fi));
    PendingGame& entry = register_game(game, &collection);
    create_game_file_for(std::move(fi), entry);
    return entry;
}

void SearchContext::remove_invalid_items()
{
    // NOTE: as Collections depend on Games, make sure Games are removed first
    std::vector<QString> affected_collections;
    std::vector<size_t> affected_games;


    auto game_it = m_games.begin();
    while (game_it != m_games.end()) {
        const PendingGame& entry = game_it->second;
        const bool has_files = !entry.files().empty();
        const bool has_colls = !entry.collection_ids().empty();
        if (has_files && has_colls) {
            ++game_it;
            continue;
        }

        if (!has_files) {
            qWarning().noquote()
                << tr_log("No files defined for game '%1', ignored").arg(entry.inner().title());
        }
        else if (!has_colls) {
            qWarning().noquote()
                << tr_log("Game '%1' does not belong to any collections, ignored").arg(entry.inner().title());
        }

        affected_collections.insert(
            affected_collections.end(),
            entry.collection_ids().cbegin(),
            entry.collection_ids().cend());
        affected_games.emplace_back(entry.id());
        game_it = m_games.erase(game_it);
    }


    VEC_REMOVE_DUPLICATES(affected_collections);
    for (const QString& coll_id : affected_collections) {
        PendingCollection& coll = m_collections.at(coll_id);
        for (const size_t game: affected_games)
            coll.m_game_ids.erase(game);
    }


    auto coll_it = m_collections.begin();
    while (coll_it != m_collections.end()) {
        PendingCollection& coll = coll_it->second;
        const bool has_games = !coll.game_ids().empty();
        if (has_games) {
            ++coll_it;
            continue;
        }

        qWarning().noquote()
            << tr_log("No valid games found for collection '%1', ignored").arg(coll.inner().name());
        coll_it = m_collections.erase(coll_it);
    }
}
} // namespace providers
