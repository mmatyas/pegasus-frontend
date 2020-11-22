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

#include "AppSettings.h"
#include "LocaleUtils.h"
#include "Log.h"
#include "model/gaming/Collection.h"
#include "model/gaming/Game.h"
#include "model/gaming/GameFile.h"
#include "utils/DiskCachedNAM.h"
#include "utils/StdHelpers.h"

#include <QFileInfo>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSslSocket>


namespace {
QStringList read_game_dirs()
{
    QStringList game_dirs;

    AppSettings::parse_gamedirs([&game_dirs](const QString& line){
        const QFileInfo finfo(line);
        if (finfo.isDir())
            game_dirs.append(finfo.canonicalFilePath());
    });

    game_dirs.removeDuplicates();
    return game_dirs;
}
} // namespace


namespace providers {
SearchContext::SearchContext(QObject* parent)
    : SearchContext(read_game_dirs(), parent)
{}

SearchContext::SearchContext(QStringList game_dirs, QObject* parent)
    : QObject(parent)
    , m_root_game_dirs(std::move(game_dirs))
    , m_netman(nullptr)
    , m_pending_downloads(0)
{}

SearchContext& SearchContext::pegasus_add_game_dir(QString path)
{
    m_pegasus_game_dirs.append(std::move(path));
    m_pegasus_game_dirs.removeDuplicates();  // TODO: optimize?
    return *this;
}

model::Collection* SearchContext::get_or_create_collection(const QString& name)
{
    const auto it = m_collections.find(name);
    if (it != m_collections.cend())
        return it->second;

    auto* const new_ptr = new model::Collection(name);
    m_collections.emplace(name, new_ptr);
    return new_ptr;
}

model::Game* SearchContext::create_game_for(model::Collection& collection)
{
    auto* const game_ptr = new model::Game();
    (*game_ptr)
        .setLaunchCmd(collection.commonLaunchCmd())
        .setLaunchWorkdir(collection.commonLaunchWorkdir())
        .setLaunchCmdBasedir(collection.commonLaunchCmdBasedir());

    m_collection_games[&collection].emplace_back(game_ptr);
    return game_ptr;
}

model::Game* SearchContext::create_game()
{
    auto* const game_ptr = new model::Game();
    m_parentless_games.emplace_back(game_ptr);
    return game_ptr;
}

model::Game* SearchContext::game_by_filepath(const QString& can_path) const
{
    model::GameFile* const entry_ptr = gamefile_by_filepath(can_path);
    return entry_ptr
        ? entry_ptr->parentGame()
        : nullptr;
}

model::Game* SearchContext::game_by_uri(const QString& uri) const
{
    model::GameFile* const entry_ptr = gamefile_by_uri(uri);
    return entry_ptr
        ? entry_ptr->parentGame()
        : nullptr;
}

model::GameFile* SearchContext::gamefile_by_filepath(const QString& can_path) const
{
    const auto it = m_filepath_to_gamefile.find(can_path);
    return it != m_filepath_to_gamefile.cend()
        ? it->second
        : nullptr;
}

model::GameFile* SearchContext::gamefile_by_uri(const QString& uri) const
{
    const auto it = m_uri_to_gamefile.find(uri);
    return it != m_uri_to_gamefile.cend()
        ? it->second
        : nullptr;
}

model::GameFile* SearchContext::game_add_filepath(model::Game& game, QString can_path)
{
    model::GameFile* const registered_ptr = gamefile_by_filepath(can_path);
    if (registered_ptr)
        return registered_ptr;

    auto* const entry_ptr = new model::GameFile(can_path, game);
    m_game_entries[&game].emplace_back(entry_ptr);
    m_filepath_to_gamefile.emplace(can_path, entry_ptr);

    if (game.title().isEmpty()) {
        game.setTitle(entry_ptr->name())
            .setSortBy(entry_ptr->name());
    }

    return entry_ptr;
}

model::GameFile* SearchContext::game_add_uri(model::Game& game, QString uri)
{
    model::GameFile* const registered_ptr = gamefile_by_uri(uri);
    if (registered_ptr)
        return registered_ptr;

    auto* const entry_ptr = new model::GameFile(uri, game);
    m_game_entries[&game].emplace_back(entry_ptr);
    m_uri_to_gamefile.emplace(uri, entry_ptr);
    return entry_ptr;
}

SearchContext& SearchContext::game_add_to(model::Game& game, model::Collection& collection)
{
    m_collection_games[&collection].emplace_back(&game);
    VEC_REMOVE_VALUE(m_parentless_games, &game);

    if (game.launchCmd().isEmpty())
        game.setLaunchCmd(collection.commonLaunchCmd());
    if (game.launchWorkdir().isEmpty())
        game.setLaunchWorkdir(collection.commonLaunchWorkdir());
    if (game.launchCmdBasedir().isEmpty())
        game.setLaunchCmdBasedir(collection.commonLaunchCmdBasedir());

    return *this;
}

std::pair<QVector<model::Collection*>, QVector<model::Game*>> SearchContext::finalize(QObject* const qparent)
{
    // TODO: C++17

    for (model::Game* const game_ptr : m_parentless_games) {
        Log::warning(tr_log("The game '%1' does not belong to any collections, ignored").arg(game_ptr->title()));
        delete game_ptr;
    }

    std::vector<model::Game*> deleted_games;
    for (const auto& pair : m_game_entries) {
        if (!pair.second.empty())
            continue;

        model::Game* const game_ptr = pair.first;
        Log::warning(tr_log("The game '%1' has no launchable entries, ignored").arg(game_ptr->title()));
        deleted_games.emplace_back(game_ptr);
        delete game_ptr;
    }

    for (auto& pair : m_collection_games) {
        for (model::Game* const game_ptr : deleted_games)
            VEC_REMOVE_VALUE(pair.second, game_ptr);

        if (!pair.second.empty())
            continue;

        model::Collection* const coll_ptr = pair.first;
        Log::warning(tr_log("The collection '%1' has no valid games, ignored").arg(coll_ptr->name()));
        m_collections.erase(coll_ptr->name());
        delete coll_ptr;
    }


    for (auto& pair : m_game_entries)
        pair.first->setFiles(std::move(pair.second));

    HashMap<model::Game*, std::vector<model::Collection*>> game_collections;
    for (auto& pair : m_collection_games) {
        for (model::Game* const game_ptr : pair.second)
            game_collections[game_ptr].emplace_back(pair.first);
    }
    for (auto& pair : game_collections) {
        VEC_REMOVE_DUPLICATES(pair.second);
        pair.first->setCollections(std::move(pair.second));
    }

    for (auto& pair : m_collection_games) {
        VEC_REMOVE_DUPLICATES(pair.second);
        pair.first->setGames(std::move(pair.second));
    }


    QVector<model::Game*> games;
    games.reserve(m_filepath_to_gamefile.size() + m_uri_to_gamefile.size());
    for (const auto& pair : m_filepath_to_gamefile)
        games.append(pair.second->parentGame());
    for (const auto& pair : m_uri_to_gamefile)
        games.append(pair.second->parentGame());
    VEC_REMOVE_DUPLICATES(games);
    games.squeeze();

    for (model::Game* const game : games) {
        game->developerList().removeDuplicates();
        game->publisherList().removeDuplicates();
        game->genreList().removeDuplicates();
        game->tagList().removeDuplicates();
    }


    QVector<model::Collection*> collections;
    collections.reserve(m_collections.size());
    for (auto& pair : m_collections)
        collections.append(pair.second);


    std::sort(collections.begin(), collections.end(), model::sort_collections);
    std::sort(games.begin(), games.end(), model::sort_games);
    for (model::Collection* const collection : collections) {
        collection->moveToThread(qparent->thread());
        collection->setParent(qparent);
    }
    for (model::Game* const game : games) {
        game->moveToThread(qparent->thread());
        game->setParent(qparent);
    }


    return std::make_pair(std::move(collections), std::move(games));
}


SearchContext& SearchContext::enable_network()
{
    Q_ASSERT(!m_netman);

    if (!QSslSocket::supportsSsl()) {
        Log::warning(tr_log("Secure connection (SSL) support not available, downloading metadata is not possible"));
        return *this;
    }

    // TODO: C++14
    m_netman = utils::create_disc_cached_nam(this);
    return *this;
}

bool SearchContext::has_network() const
{
    return !!m_netman;
}

bool SearchContext::has_pending_downloads() const
{
    return m_pending_downloads > 0;
}

SearchContext& SearchContext::schedule_download(
    const QUrl& url,
    const std::function<void(QNetworkReply* const)>& on_finish_callback)
{
    Q_ASSERT(m_netman);
    Q_ASSERT(url.isValid());

    m_pending_downloads++;

    QNetworkRequest request(url);
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    request.setTransferTimeout(10000);
#endif

    QNetworkReply* const reply = m_netman->get(request);
    emit downloadScheduled();

    QObject::connect(reply, &QNetworkReply::finished,
        this, [this, reply, on_finish_callback]{
            on_finish_callback(reply);
            m_pending_downloads--;
            emit downloadCompleted();
        });

    return *this;
}
} // namespace providers
