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


#include "Game.h"

#include "model/gaming/Assets.h"
#include "model/gaming/Collection.h"
#include "model/gaming/GameFile.h"


namespace {
QString joined_list(const QStringList& list) { return list.join(QLatin1String(", ")); }
} // namespace


namespace model {
GameData::GameData() = default;

GameData::GameData(QString new_title)
    : title(std::move(new_title))
    , sort_by(title)
{}


Game::Game(QString name, QObject* parent)
    : QObject(parent)
    , m_data(std::move(name))
    , m_assets(new model::Assets(this))
{}

Game::Game(QObject* parent)
    : Game(QString(), parent)
{}

QString Game::developerStr() const { return joined_list(m_data.developers); }
QString Game::publisherStr() const { return joined_list(m_data.publishers); }
QString Game::genreStr() const { return joined_list(m_data.genres); }
QString Game::tagStr() const { return joined_list(m_data.tags); }

Game& Game::setTitle(QString title)
{
    m_data.title = std::move(title);
    if (sortBy().isEmpty())
        setSortBy(m_data.title);
    return *this;
}

Game& Game::setUri(QString uri)
{
    m_data.uri = std::move(uri);
    return *this;
}

Game& Game::setFavorite(bool new_val)
{
    m_data.is_favorite = new_val;
    emit favoriteChanged();
    return *this;
}

Game& Game::setMissing(bool new_val)
{
    m_data.missing = new_val;
    return *this;
}

Game& Game::setRating(float rating)
{
    m_data.rating = qBound(0.f, rating, 1.f);
    return *this;
}

Game& Game::setPlayerCount(int player_count)
{
    m_data.player_count = std::max(1, player_count);
    return *this;
}

void Game::update_playstats(int playcount, qint64 playtime, QDateTime last_played)
{
    m_data.playstats.last_played = std::max(m_data.playstats.last_played, std::move(last_played));
    m_data.playstats.play_time += playtime;
    m_data.playstats.play_count += playcount;
    emit playStatsChanged();
}

void Game::launch()
{
    if (m_files->isEmpty())
        emit launchRequested();
    if (m_files->count() == 1)
        m_files->entries().front()->launch();
    else
        emit launchFileSelectorRequested();
}

Game& Game::setFiles(std::vector<model::GameFile*>&& files)
{
    for (model::GameFile* const gamefile : files) {
        connect(this, &model::Game::playStatsChanged,
                gamefile, &model::GameFile::playStatsChanged);
    }

    std::sort(files.begin(), files.end(), model::sort_gamefiles);

    Q_ASSERT(!m_files);
    m_files = new GameFileListModel(this);
    m_files->update(std::move(files));

    return *this;
}

Game& Game::setCollections(std::vector<model::Collection*>&& collections)
{
    std::sort(collections.begin(), collections.end(), model::sort_collections);

    Q_ASSERT(!m_collections);
    m_collections = new CollectionListModel(this);
    m_collections->update(std::move(collections));
    return *this;
}

bool sort_games(const model::Game* const a, const model::Game* const b) {
   return QString::localeAwareCompare(a->sortBy(), b->sortBy()) < 0;
}
} // namespace model
