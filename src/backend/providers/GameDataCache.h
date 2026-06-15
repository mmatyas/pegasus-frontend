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

#include <QString>
#include <QStringList>
#include <vector>

namespace model {
class Collection;
class Game;
}

namespace providers {
class Provider;
class SearchContext;
}

class GameDataCache {
public:
    static bool load(
        providers::SearchContext&,
        const std::vector<providers::Provider*>&);

    static void save(
        const providers::SearchContext&,
        const std::vector<providers::Provider*>&,
        const std::vector<model::Collection*>&,
        const std::vector<model::Game*>&);

    static void clear();

private:
    static QString cacheFilePath();
    static QString buildFingerprint(
        const providers::SearchContext&,
        const std::vector<providers::Provider*>&);
};