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

#include <QVector>

namespace Model { class Platform; }


namespace model_providers {

class PlatformListProvider {
public:
    PlatformListProvider() {}
    virtual ~PlatformListProvider() {}

    /// Find the available platforms, along with their essential
    /// informations, such as game/rom paths or supported file extensions.
    /// Do not return null pointers.
    virtual QVector<Model::Platform*> find() = 0;

    // disable copy
    PlatformListProvider(const PlatformListProvider&) = delete;
    PlatformListProvider& operator=(const PlatformListProvider&) = delete;
};

} // namespace model_providers
