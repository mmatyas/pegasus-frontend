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


#include "Provider.h"

#include "model/gaming/Game.h"
#include "model/gaming/Collection.h"

namespace {
void add_new_game_to_parent_maybe(model::Game* game, model::Collection* collection)
{
    if (collection) {
        game->addCollection(collection)
            .setLaunchCmd(collection->commonLaunchCmd())
            .setLaunchWorkdir(collection->commonLaunchWorkdir())
            .setLaunchCmdBasedir(collection->commonLaunchCmdBasedir());
        collection->addGame(game);
    }
}
} // namespace


namespace providers {
std::pair<size_t, model::Game*> SearchContext::new_game(QFileInfo fi, model::Collection* collection)
{
    QString file_path = fi.canonicalFilePath();

    auto game = new model::Game(std::move(fi));
    add_new_game_to_parent_maybe(game, collection);

    size_t game_id = games.size();
    path_to_gameid.emplace(file_path, game_id);
    games.emplace(game_id, game);

    return std::make_pair(game_id, game);
}

std::pair<size_t, model::Game*> SearchContext::new_empty_game(QString name, model::Collection* collection)
{
    auto game = new model::Game(name);
    add_new_game_to_parent_maybe(game, collection);

    size_t game_id = games.size();
    games.emplace(game_id, game);

    return std::make_pair(game_id, game);
}

std::pair<size_t, model::Game*> SearchContext::get_or_create_game(QFileInfo fi)
{
    QString file_path = fi.canonicalFilePath();
    Q_ASSERT(!file_path.isEmpty());

    auto found = path_to_gameid.find(file_path);
    if (found != path_to_gameid.end()) {
        Q_ASSERT(games.count(found->second));
        return std::make_pair(found->second, games[found->second]);
    }

    return new_game(std::move(fi), nullptr);
}

std::pair<size_t, model::Game*> SearchContext::add_or_create_game_for(QFileInfo fi, model::Collection& collection)
{
    QString file_path = fi.canonicalFilePath();
    Q_ASSERT(!file_path.isEmpty());

    auto found = path_to_gameid.find(file_path);
    if (found != path_to_gameid.end()) {
        Q_ASSERT(games.count(found->second));
        model::Game* game = games[found->second];
        collection.addGame(game);
        game->addCollection(&collection);
        return std::make_pair(found->second, game);
    }

    return new_game(std::move(fi), &collection);
}

model::Collection* SearchContext::get_or_create_collection(QString name)
{
    auto found = collections.find(name);
    if (found != collections.end())
        return found->second;

    auto coll = new model::Collection(name);
    collections.emplace(std::move(name), coll);
    return coll;
}


Provider::Provider(QLatin1String codename, QString name, uint8_t flags, QObject* parent)
    : QObject(parent)
    , m_codename(std::move(codename))
    , m_provider_name(std::move(name))
    , m_provider_flags(flags)
    , m_enabled(true)
{}

Provider::~Provider() = default;

Provider& Provider::setEnabled(bool val)
{
    m_enabled = val;
    return *this;
}

Provider& Provider::setOption(const QString& key, QString val)
{
    Q_ASSERT(!key.isEmpty());
    Q_ASSERT(!val.isEmpty());
    setOption(key, std::vector<QString>{ std::move(val) });
    return *this;
}

Provider& Provider::setOption(const QString& key, std::vector<QString> vals)
{
    Q_ASSERT(!key.isEmpty());
    Q_ASSERT(!vals.empty());
    m_options[key] = std::move(vals);
    return *this;
}

} // namespace providers
