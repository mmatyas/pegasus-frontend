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

namespace Types { class Game; }
namespace Types { class Platform; }


namespace model_providers {

class MetadataProvider {
public:
    MetadataProvider() {}
    virtual ~MetadataProvider() {}

    /// You get a Platform, which already has its list of games found,
    /// but they may miss some or all of their metadata (including assets).
    /// Fill the metadata fields and find the missing assets!
    ///
    /// NOTE: while the Platform itself is const, the data pointed by
    ///       the Game pointers is not!
    virtual void fill(const Types::Platform&) = 0;

    // disable copy
    MetadataProvider(const MetadataProvider&) = delete;
    MetadataProvider& operator=(const MetadataProvider&) = delete;
};

} // namespace model_providers
