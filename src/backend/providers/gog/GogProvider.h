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


#pragma once

#include "GogGamelist.h"
#include "GogMetadata.h"
#include "providers/Provider.h"
#include "utils/HashMap.h"


namespace providers {
namespace gog {

class GogProvider : public Provider {
    Q_OBJECT

public:
    explicit GogProvider(QObject* parent = nullptr);

    Provider& findLists(SearchContext&) final;
    Provider& findStaticData(SearchContext&) final;

private:
    HashMap<size_t, QString> m_gogids;

    Gamelist gamelist;
    Metadata metadata;
};

} // namespace gog
} // namespace providers
