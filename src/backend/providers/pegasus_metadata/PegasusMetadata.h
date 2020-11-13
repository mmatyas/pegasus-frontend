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
#include "utils/NoCopyNoMove.h"

#include <QDir>
#include <QString>
#include <QRegularExpression>

namespace metafile { struct Entry; }
namespace metafile { struct Error; }
namespace model { class Game; }
namespace model { class Collection; }
namespace providers { class SearchContext; }


namespace providers {
namespace pegasus {

enum class CollAttrib : unsigned char;
enum class GameAttrib : unsigned char;
struct FileFilter;


struct ParserState {
    const QString& path;
    const QDir dir;
    model::Game* cur_game = nullptr;
    model::Collection* cur_coll = nullptr;
    std::vector<FileFilter> filters;

    explicit ParserState(const QString&);
    NO_COPY_NO_MOVE(ParserState)
};


class Metadata {
public:
    explicit Metadata(QString);

    std::vector<FileFilter> apply_metafile(const QString&, SearchContext&) const;

private:
    const QString m_log_tag;

    const QLatin1String m_primary_key_collection;
    const QLatin1String m_primary_key_game;

    const HashMap<QString, CollAttrib> m_coll_attribs;
    const HashMap<QString, GameAttrib> m_game_attribs;
    //const HashMap<QString, GameFileAttrib> gamefile_attribs;

    const QRegularExpression rx_asset_key;
    const QRegularExpression rx_count_range;
    const QRegularExpression rx_percent;
    const QRegularExpression rx_float;
    const QRegularExpression rx_date;
    const QRegularExpression rx_unescaped_newline;


    void print_error(const ParserState& ps, const metafile::Error&) const;
    void print_warning(const ParserState& ps, const metafile::Entry&, const QString&) const;

    const QString& first_line_of(const ParserState& ps, const metafile::Entry&) const;
    void replace_newlines(QString&) const;

    void apply_collection_entry(ParserState&, const metafile::Entry&) const;
    void apply_game_entry(ParserState&, const metafile::Entry&, SearchContext&) const;
    bool apply_asset_entry_maybe(ParserState&, const metafile::Entry&) const;
    void apply_entry(ParserState&, const metafile::Entry&, SearchContext&) const;
};

} // namespace pegasus
} // namespace providers
