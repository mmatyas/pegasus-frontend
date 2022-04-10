// Pegasus Frontend
// Copyright (C) 2017-2022  Mátyás Mustoha
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


#include "GameFileListModel.h"

#include "model/gaming/Assets.h"
#include "model/gaming/GameFile.h"


namespace model {
enum GameFileRoles {
    Self = Qt::UserRole,
    Name,
    Path,
    PlayCount,
    PlayTime,
    LastPlayed,
};
using Roles = GameFileRoles;


GameFileListModel::GameFileListModel(QObject* parent)
    : QAbstractListModel(parent)
{}


QHash<int, QByteArray> GameFileListModel::roleNames() const
{
    static QHash<int, QByteArray> ROLE_NAMES {
        { Roles::Self, QByteArrayLiteral("modelData") },
        { Roles::Name, QByteArrayLiteral("name") },
        { Roles::Path, QByteArrayLiteral("path") },
        { Roles::PlayCount, QByteArrayLiteral("playCount") },
        { Roles::PlayTime, QByteArrayLiteral("playTime") },
        { Roles::LastPlayed, QByteArrayLiteral("lastPlayed") },
    };
    return ROLE_NAMES;
}


GameFileListModel& GameFileListModel::update(std::vector<model::GameFile*>&& entries)
{
    const bool count_changed = m_entries.size() != entries.size();

    beginResetModel();
    for (model::GameFile* game : m_entries)
        disconnect(game, nullptr, this, nullptr);

    m_entries = std::move(entries);

    for (model::GameFile* game : m_entries) {
        connect(game, &model::GameFile::playStatsChanged,
                this, [this](){ onEntryPropertyChanged({Roles::PlayCount, Roles::PlayTime, Roles::LastPlayed}); });
    }
    endResetModel();

    if (count_changed)
        emit countChanged();

    return *this;
}


int GameFileListModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : m_entries.size();
}


QVariant GameFileListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return {};

    model::GameFile* const gamefile_ptr = m_entries.at(index.row());
    const model::GameFile& gamefile = *gamefile_ptr;
    switch (role) {
        case Roles::Self: return QVariant::fromValue(gamefile_ptr);
        case Roles::Name: return gamefile.name();
        case Roles::Path: return gamefile.path();
        case Roles::PlayCount: return gamefile.playCount();
        case Roles::PlayTime: return gamefile.playTime();
        case Roles::LastPlayed: return gamefile.lastPlayed();
        default: return {};
    }
}


QVariantList GameFileListModel::toVarArray() const
{
    QVariantList varlist;
    varlist.reserve(m_entries.size());
    for (model::GameFile* ptr : m_entries)
        varlist.append(QVariant::fromValue(ptr));
    return varlist;
}


model::GameFile* GameFileListModel::get(int idx) const
{
    return (0 <= idx && static_cast<size_t>(idx) < m_entries.size())
        ? m_entries.at(idx)
        : nullptr;
}


void GameFileListModel::onEntryPropertyChanged(const QVector<int>& roles)
{
    QObject* const game_ptr = sender();
    const auto it = std::find(m_entries.cbegin(), m_entries.cend(), game_ptr);
    if (it == m_entries.cend())
       return;

    const size_t data_idx = std::distance(m_entries.cbegin(), it);
    const QModelIndex model_idx = index(data_idx);
    emit dataChanged(model_idx, model_idx, roles);
}
} // namespace model
