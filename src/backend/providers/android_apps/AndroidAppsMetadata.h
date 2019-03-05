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

#include "providers/Provider.h"
#include "utils/HashMap.h"

#include <QRegularExpression>


namespace providers {
namespace android {

class Metadata {
public:
    Metadata();

    void findStaticData(SearchContext&);

private:
    const QRegularExpression rx_meta_itemprops;
    const QRegularExpression rx_background;
    const QRegularExpression rx_developer;
    const QRegularExpression rx_category;
    const QRegularExpression rx_screenshots;

    std::vector<size_t> fill_from_cache(const std::vector<size_t>&, HashMap<size_t, modeldata::Game>&);
    void fill_from_network(const std::vector<size_t>&, HashMap<size_t, modeldata::Game>&);
    bool parse_reply(QByteArray&, QJsonObject&);
};

} // namespace android
} // namespace providers
