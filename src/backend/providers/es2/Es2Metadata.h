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
#include "types/AssetType.h"

#include <QString>
#include <QRegularExpression>

namespace providers { class SearchContext; }
namespace model { class GameFile; }
class QDir;
class QXmlStreamReader;


namespace providers {
namespace es2 {

struct SystemEntry;
enum class MetaType : unsigned char;

class Metadata {

public:
    explicit Metadata(QString);
    void find_metadata_for(const SystemEntry&, const SearchContext&) const;

private:
    const QString m_log_tag;
    const HashMap<QString, MetaType> m_key_types;
    const QString m_date_format;
    const QRegularExpression m_players_regex;
    const std::vector<std::pair<MetaType, AssetType>> m_asset_type_map;

    void process_gamelist_xml(const QDir&, QXmlStreamReader&, const SearchContext&) const;
    HashMap<MetaType, QString, EnumHash> parse_gamelist_game_node(QXmlStreamReader&) const;
    void apply_metadata(model::GameFile&, const QDir&, HashMap<MetaType, QString, EnumHash>&) const;
};

} // namespace es2
} // namespace providers
