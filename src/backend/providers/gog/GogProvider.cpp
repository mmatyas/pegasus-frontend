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


#include "GogProvider.h"


namespace providers {
namespace gog {

GogProvider::GogProvider(QObject* parent)
    : Provider(parent)
    , gamelist(this)
    , metadata(this)
{
    connect(&gamelist, &Gamelist::gameCountChanged,
            this, &GogProvider::gameCountChanged);
}

void GogProvider::findLists(SearchContext& sctx)
{
    gamelist.find(sctx, m_gogids);
}

void GogProvider::findStaticData(SearchContext& sctx)
{
    metadata.enhance(sctx, m_gogids);
}

} // namespace gog
} // namespace providers
