// Pegasus Frontend
// Copyright (C) 2017-2020  Mátyás Mustoha
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

#include "Log.h"
#include "GogGamelist.h"
#include "GogMetadata.h"
#include "providers/SearchContext.h"

#include <QSslSocket>


namespace {
void fill_metadata_from_cache(HashMap<QString, model::Game*>& gogid_game_map, const providers::gog::Metadata& metahelper)
{
    std::vector<QString> found_gogids;
    found_gogids.reserve(gogid_game_map.size());

    // TODO: C++17
    for (const auto& entry : gogid_game_map) {
        const QString& gogid = entry.first;
        model::Game& game = *entry.second;

        const bool found = metahelper.fill_from_cache(gogid, game);
        if (found)
            found_gogids.emplace_back(gogid);
    }

    for (const QString& gogid : found_gogids)
        gogid_game_map.erase(gogid);
}

void fill_metadata_from_network(
    const HashMap<QString, model::Game*>& gogid_game_map,
    const providers::gog::Metadata& metahelper,
    providers::SearchContext& sctx)
{
    if (gogid_game_map.empty())
        return;

    if (!sctx.has_network())
        return;

    // TODO: C++17
    for (const auto& entry : gogid_game_map) {
        const QString& gogid = entry.first;
        model::Game* const game = entry.second;
        metahelper.fill_from_network(gogid, *game, sctx);
    }
}
} // namespace


namespace providers {
namespace gog {

GogProvider::GogProvider(QObject* parent)
    : Provider(QLatin1String("gog"), QStringLiteral("GOG"), parent)
{}

Provider& GogProvider::run(SearchContext& sctx)
{
    model::Collection& collection = *sctx.get_or_create_collection(QStringLiteral("GOG"));

    HashMap<QString, model::Game*> gogid_game_map = Gamelist(display_name()).find(options(), collection, sctx);
    if (gogid_game_map.empty())
        return *this;

    const Metadata metahelper(display_name());
    fill_metadata_from_cache(gogid_game_map, metahelper);
    fill_metadata_from_network(gogid_game_map, metahelper, sctx);

    return *this;
}

} // namespace gog
} // namespace providers
