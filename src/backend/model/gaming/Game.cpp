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


#include "Game.h"


namespace {
QString joined_list(const QStringList& list) { return list.join(QLatin1String(", ")); }
} // namespace


namespace model {
GameData::GameData(QString new_title)
    : title(std::move(new_title))
    , sort_title(title)
{}


Game::Game(QFileInfo fi, QObject* parent)
    : Game(pretty_filename(fi), parent)
{
    createFile(std::move(fi), title());
}

Game::Game(QString name, QObject* parent)
    : QObject(parent)
    , m_files(new QQmlObjectListModel<model::GameFile>(this))
    , m_collections(new QQmlObjectListModel<model::Collection>(this))
    , m_data(name)
    , m_assets(new model::Assets(this))
{}

QString Game::developerStr() const { return joined_list(m_data.developers); }
QString Game::publisherStr() const { return joined_list(m_data.publishers); }
QString Game::genreStr() const { return joined_list(m_data.genres); }
QString Game::tagStr() const { return joined_list(m_data.tags); }

Game& Game::setFavorite(bool new_val)
{
    m_data.is_favorite = new_val;
    emit favoriteChanged();
    return *this;
}

Game& Game::addCollection(model::Collection* ptr)
{
    Q_ASSERT(ptr);
    m_collection_set.insert(ptr);
    return *this;
}

Game& Game::addFile(model::GameFile* ptr)
{
    Q_ASSERT(ptr);
    m_file_set.insert(ptr);
    return *this;
}

Game& Game::createFile(QFileInfo fi)
{
    return addFile(new model::GameFile(std::move(fi), this));
}

Game& Game::createFile(QFileInfo fi, QString name)
{
    return addFile(new model::GameFile(std::move(fi), std::move(name), this));
}

void Game::onEntryPlayStatsChanged()
{
    m_data.playstats.play_count = std::accumulate(filesConst().cbegin(), filesConst().cend(), 0,
        [](int sum, const model::GameFile* const gamefile){
            return sum + gamefile->playCount();
        });
    m_data.playstats.play_time = std::accumulate(filesConst().cbegin(), filesConst().cend(), 0,
        [](qint64 sum, const model::GameFile* const gamefile){
            return sum + gamefile->playTime();
        });
    m_data.playstats.last_played = std::accumulate(filesConst().cbegin(), filesConst().cend(), QDateTime(),
        [](QDateTime current_max, const model::GameFile* const gamefile){
            return std::max(current_max, gamefile->lastPlayed());
        });

    emit playStatsChanged();
}

void Game::launch()
{
    Q_ASSERT(m_files->count() > 0);

    if (m_files->count() == 1)
        m_files->first()->launch();
    else
        emit launchFileSelectorRequested();
}

void Game::finalize()
{
    for (model::GameFile* const gamefile : m_file_set) {
        connect(gamefile, &model::GameFile::playStatsChanged,
                this, &model::Game::onEntryPlayStatsChanged);
    }

    // TODO: C++17 set.extract
    QVector<model::Collection*> coll_vec;
    coll_vec.reserve(m_collection_set.size());
    std::copy(m_collection_set.begin(), m_collection_set.end(), std::back_inserter(coll_vec));
    std::sort(coll_vec.begin(), coll_vec.end(), model::sort_collections);
    m_collections->append(std::move(coll_vec));
    m_collection_set.clear();

    QVector<model::GameFile*> file_vec;
    file_vec.reserve(m_file_set.size());
    std::copy(m_file_set.begin(), m_file_set.end(), std::back_inserter(file_vec));
    std::sort(file_vec.begin(), file_vec.end(), model::sort_gamefiles);
    m_files->append(std::move(file_vec));
    m_file_set.clear();
}

bool sort_games(const model::Game* const a, const model::Game* const b) {
   return QString::localeAwareCompare(a->sortTitle(), b->sortTitle()) < 0;
};
} // namespace model
