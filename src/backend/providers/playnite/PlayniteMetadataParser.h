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

#include <QDir>

namespace providers {
namespace playnite {
struct PlayniteGame;
struct PlayniteEmulator;
struct PlayniteComponents;

class PlayniteMetadataParser {
public:
    PlayniteMetadataParser(QString log_tag, const QDir& playnite_dir);
    PlayniteComponents parse_metadata() const;

private:
    const QString m_log_tag;
    const QDir m_playnite_dir;
    const QDir::Filters m_dir_filters;
    const QStringList m_json_ext_list;

    HashMap<QString, QString> parse_platform_metadata() const;
    HashMap<QString, QString> parse_source_metadata() const;
    HashMap<QString, QString> parse_company_metadata() const;
    HashMap<QString, QString> parse_genre_metadata() const;
    HashMap<QString, PlayniteEmulator> parse_emulator_metadata() const;
    std::vector<PlayniteGame> parse_game_metadata() const;
    QJsonObject get_json_object_from_file(const QString& file_path) const;
    HashMap<QString, QString> parse_id_name_files(const QString& rel_path) const;
};

} // namespace playnite
} // namespace providers
