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


#include "GameFile.h"


namespace model {
GameFile::GameFile(modeldata::GameFile data, QObject* parent)
    : QObject(parent)
    , m_data(std::move(data))
{}

void GameFile::launch()
{
    emit launchRequested();
}

// This one is for summing the play times provided by multiple Providers.
void GameFile::addPlayStats(int playcount, qint64 playtime, const QDateTime& last_played)
{
    m_data.last_played = std::max(m_data.last_played, last_played);
    m_data.play_time += playtime;
    m_data.play_count += playcount;
    emit playStatsChanged();
}

// This one is a single update for playtime when the game finishes.
void GameFile::updatePlayTime(qint64 duration, QDateTime time_finished)
{
    m_data.last_played = std::move(time_finished);
    m_data.play_time += duration;
    m_data.play_count++;
    emit playStatsChanged();
}
} // namespace model
