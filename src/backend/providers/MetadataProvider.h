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


#pragma once

#include <QHash>
#include <QVector>
#include <QString>

namespace Types { class Collection; }
namespace Types { class Game; }


namespace providers {

class MetadataProvider {
public:
    MetadataProvider() = default;
    virtual ~MetadataProvider() = default;

    virtual void fill(const QHash<QString, Types::Game*>& games,
                      const QHash<QString, Types::Collection*>& collections,
                      const QVector<QString>& metadata_dirs) = 0;

    // disable copy
    MetadataProvider(const MetadataProvider&) = delete;
    MetadataProvider& operator=(const MetadataProvider&) = delete;
};

} // namespace providers
