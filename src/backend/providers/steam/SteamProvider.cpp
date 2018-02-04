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


#include "SteamProvider.h"


namespace providers {
namespace steam {

SteamProvider::SteamProvider(QObject* parent)
    : Provider(parent)
    , gamelist(this)
    , metadata(this)
{
    connect(&gamelist, &Gamelist::gameCountChanged,
            this, &SteamProvider::gameCountChanged);
}

void SteamProvider::find(QHash<QString, Types::Game*>& games,
                         QHash<QString, Types::Collection*>& collections)
{
    gamelist.find(games, collections);
}

void SteamProvider::enhance(const QHash<QString, Types::Game*>& games,
                            const QHash<QString, Types::Collection*>& collections)
{
    metadata.enhance(games, collections);
}

} // namespace steam
} // namespace providers
