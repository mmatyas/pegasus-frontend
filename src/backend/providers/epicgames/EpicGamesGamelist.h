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


#pragma once

#include "utils/HashMap.h"

#include <QRegularExpression>
#include <QStringList>

namespace model { class Game; }
namespace model { class Collection; }
namespace providers { class SearchContext; }


namespace providers {
namespace epicgames {

class Gamelist {
public:
    explicit Gamelist(QString);

    HashMap<QString, model::Game*> find_in(const QString&, const QString&, model::Collection&, SearchContext&) const;

private:
    const QString m_log_tag;
    const QStringList m_name_filters;
    const std::vector<QLatin1String> m_ignored_manifests;
    const QRegularExpression m_rx_manifest_catalog_app_name;
    const QRegularExpression m_rx_manifest_title;

    std::tuple<QString, QString> read_manifest_file(const QString&) const;
};

} // namespace epicgames
} // namespace providers
