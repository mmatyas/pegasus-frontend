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


#include "PegasusMetadataParser.h"

#include "LocaleUtils.h"
#include "PegasusAssets.h"
#include "PegasusMetadataConstants.h"
#include "PegasusMetadataFilter.h"
#include "PegasusUtils.h"
#include "model/gaming/Collection.h"
#include "model/gaming/Game.h"
#include "parsers/MetaFile.h"
#include "utils/StdHelpers.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QStringBuilder>


namespace {
static constexpr auto MSG_PREFIX = "Metafiles:";

const QString& first_line_of(const metafile::Entry& entry)
{
    Q_ASSERT(!entry.key.isEmpty());
    Q_ASSERT(!entry.values.empty());

    if (entry.values.size() > 1) {
        qWarning().noquote() << MSG_PREFIX
            << tr_log("expected single line value for `%1` but got more. The rest of the lines will be ignored.")
               .arg(entry.key);
    }

    return entry.values.front();
}
} // namespace


namespace providers {
namespace pegasus {
namespace parser {

Parser::Parser(QString file_path, const Constants& constants)
    : m_metafile_path(std::move(file_path))
    , m_dir_path(QFileInfo(m_metafile_path).path())
    , m_constants(constants)
    , m_cur_coll(nullptr)
    , m_cur_game(nullptr)
    , m_cur_filter(nullptr)
{
    Q_ASSERT(!m_metafile_path.isEmpty());
    Q_ASSERT(!m_dir_path.isEmpty());
}

void Parser::print_error(const size_t lineno, const QString& msg) const {
    qWarning().noquote() << MSG_PREFIX
        << tr_log("`%1`, line %2: %3").arg(m_metafile_path, QString::number(lineno), msg);
}

// FIXME: don't copy
QString Parser::replace_newlines(QString str) const
{
    return str
        .replace(m_constants.rx_linebreak, QStringLiteral("\n"))
        .replace(QLatin1String(R"(\\n)"), QLatin1String(R"(\n)"));
}

void Parser::parse_collection_entry(const metafile::Entry& entry) const
{
    Q_ASSERT(m_cur_coll);
    Q_ASSERT(m_cur_filter);
    Q_ASSERT(!m_cur_game);

    if (!m_constants.coll_attribs.count(entry.key)) {
        print_error(entry.line, tr_log("unrecognized collection property `%1`, ignored").arg(entry.key));
        return;
    }

    FileFilterGroup& filter_group = entry.key.startsWith(QLatin1String("ignore-"))
        ? m_cur_filter->exclude
        : m_cur_filter->include;

    switch (m_constants.coll_attribs.at(entry.key)) {
        case CollAttrib::SHORT_NAME:
            m_cur_coll->setShortName(first_line_of(entry));
            break;
        case CollAttrib::LAUNCH_CMD:
            m_cur_coll->setCommonLaunchCmd(metafile::merge_lines(entry.values));
            break;
        case CollAttrib::LAUNCH_WORKDIR:
            m_cur_coll->setCommonLaunchWorkdir(first_line_of(entry));
            break;
        case CollAttrib::DIRECTORIES:
            for (const QString& value : entry.values) {
                const QFileInfo finfo(m_dir_path, value);
                QString can_path = finfo.canonicalFilePath();
                if (can_path.isEmpty())
                    print_error(entry.line, tr_log("directory path `%1` not found").arg(finfo.absoluteFilePath()));
                else
                    m_cur_filter->directories.emplace_back(std::move(can_path));
            }
            break;
        case CollAttrib::EXTENSIONS:
            vec_append_move(filter_group.extensions, utils::tokenize_by_comma(first_line_of(entry).toLower()));
            break;
        case CollAttrib::FILES:
            for (const QString& path : entry.values)
                filter_group.files.emplace_back(std::move(path));
            break;
        case CollAttrib::REGEX:
            filter_group.regex.setPattern(first_line_of(entry));
            if (!filter_group.regex.isValid()) {
                print_error(entry.line, tr_log("invalid regular expression: %1")
                    .arg(filter_group.regex.errorString()));
            }
            break;
        case CollAttrib::SHORT_DESC:
            m_cur_coll->setSummary(replace_newlines(metafile::merge_lines(entry.values)));
            break;
        case CollAttrib::LONG_DESC:
            m_cur_coll->setDescription(replace_newlines(metafile::merge_lines(entry.values)));
            break;
        case CollAttrib::SORT_NAME:
            m_cur_coll->setSortName(first_line_of(entry));
            break;
    }
}

void Parser::parse_game_entry(const metafile::Entry& entry, providers::SearchContext& sctx) const
{
    // NOTE: m_cur_coll may be null (ie. a game entry defined before any collection)
    Q_ASSERT(m_cur_game);

    if (!m_constants.game_attribs.count(entry.key)) {
        print_error(entry.line, tr_log("unrecognized game property `%1`, ignored").arg(entry.key));
        return;
    }

    switch (m_constants.game_attribs.at(entry.key)) {
        case GameAttrib::FILES:
            {
                const size_t game_id = sctx.games.size() - 1; // FIXME: improve this

                for (const QString& line : entry.values) {
                    QFileInfo fi(m_dir_path, line);

                    QString path = fi.canonicalFilePath();
                    if (path.isEmpty()) {
                        print_error(entry.line, tr_log("missing file `%1`").arg(line));
                        continue;
                    }
                    if (sctx.path_to_gameid.count(path) && sctx.path_to_gameid.at(path) == game_id) {
                        print_error(entry.line, tr_log("duplicate file `%1`").arg(line));
                        continue;
                    }

                    // NOTE: the case when a file is set for multiple games
                    // is not handled at the moment
                    sctx.path_to_gameid.emplace(std::move(path), game_id);
                    m_cur_game->addFile(std::move(fi));
                }
            }
            break;
        case GameAttrib::DEVELOPERS:
            for (const QString& line : entry.values)
                m_cur_game->developerList().append(line);
            break;
        case GameAttrib::PUBLISHERS:
            for (const QString& line : entry.values)
                m_cur_game->publisherList().append(line);
            break;
        case GameAttrib::GENRES:
            for (const QString& line : entry.values)
                m_cur_game->genreList().append(line);
            break;
        case GameAttrib::TAGS:
            for (const QString& line : entry.values)
                m_cur_game->tagList().append(line);
            break;
        case GameAttrib::PLAYER_COUNT:
            {
                const auto rx_match = m_constants.rx_count_range.match(first_line_of(entry));
                if (rx_match.hasMatch()) {
                    const short a = rx_match.capturedRef(1).toShort();
                    const short b = rx_match.capturedRef(3).toShort();
                    m_cur_game->setPlayerCount(std::max({static_cast<short>(1), a, b}));
                }
            }
            break;
        case GameAttrib::SHORT_DESC:
            m_cur_game->setSummary(replace_newlines(metafile::merge_lines(entry.values)));
            break;
        case GameAttrib::LONG_DESC:
            m_cur_game->setDescription(replace_newlines(metafile::merge_lines(entry.values)));
            break;
        case GameAttrib::RELEASE:
            {
                const auto rx_match = m_constants.rx_date.match(first_line_of(entry));
                if (!rx_match.hasMatch()) {
                    print_error(entry.line, tr_log("incorrect date format, should be YYYY, YYYY-MM or YYYY-MM-DD"));
                    return;
                }

                const int y = qMax(1, rx_match.captured(1).toInt());
                const int m = qBound(1, rx_match.captured(3).toInt(), 12);
                const int d = qBound(1, rx_match.captured(5).toInt(), 31);
                m_cur_game->setReleaseDate(QDate(y, m, d));
            }
            break;
        case GameAttrib::RATING:
            {
                const QString& line = first_line_of(entry);

                const auto rx_match_a = m_constants.rx_percent.match(line);
                if (rx_match_a.hasMatch()) {
                    m_cur_game->setRating(qBound(0.f, line.leftRef(line.length() - 1).toFloat() / 100.f, 1.f));
                    return;
                }
                const auto rx_match_b = m_constants.rx_float.match(line);
                if (rx_match_b.hasMatch()) {
                    m_cur_game->setRating(qBound(0.f, line.toFloat(), 1.f));
                    return;
                }

                print_error(entry.line, tr_log("failed to parse rating value"));
            }
            break;
        case GameAttrib::LAUNCH_CMD:
            m_cur_game->setLaunchCmd(metafile::merge_lines(entry.values));
            break;
        case GameAttrib::LAUNCH_WORKDIR:
            m_cur_game->setLaunchWorkdir(first_line_of(entry));
            break;
        case GameAttrib::SORT_TITLE:
            m_cur_game->setSortTitle(first_line_of(entry));
            break;
    }
}

// Returns true if the entry key matches asset regex.
// The actual asset type check may still fail however.
bool Parser::parse_asset_entry_maybe(const metafile::Entry& entry) const
{
    Q_ASSERT(m_cur_coll || m_cur_game);

    const auto rx_match = m_constants.rx_asset_key.match(entry.key);
    if (!rx_match.hasMatch())
        return false;

    const QString asset_key = rx_match.captured(1);
    const AssetType asset_type = pegasus_assets::str_to_type(asset_key);
    if (asset_type == AssetType::UNKNOWN) {
        print_error(entry.line, tr_log("unknown asset type '%1', entry ignored").arg(asset_key));
        return true;
    }

    model::Assets& assets = m_cur_game
        ? m_cur_game->assets()
        : m_cur_coll->assets();
    for (const QString& line : entry.values)
        assets.add_url(asset_type, utils::assetline_to_url(line, m_dir_path));

    return true;
}

void Parser::parse_entry(const metafile::Entry& entry,
                         providers::SearchContext& sctx,
                         std::vector<FileFilter>& filters)
{
    // TODO: set cur_* by the return value of emplace
    if (entry.key == QLatin1String("collection")) {
        const QString& name = first_line_of(entry);

        if (!sctx.collections.count(name))
            sctx.collections.emplace(name, new model::Collection(name));

        m_cur_coll = sctx.collections.at(name);
        m_cur_coll->setCommonLaunchCmdBasedir(m_dir_path);
        m_cur_game = nullptr;

        filters.emplace_back(name, m_dir_path);
        m_cur_filter = &filters.back();
        return;
    }

    if (entry.key == QLatin1String("game")) {
        auto slot = sctx.new_empty_game(first_line_of(entry), m_cur_coll);
        m_cur_game = slot.second;
        m_cur_game->setLaunchCmdBasedir(m_dir_path);
        return;
    }


    if (!m_cur_coll && !m_cur_game) {
        print_error(entry.line, tr_log("no `collection` or `game` defined yet, entry ignored"));
        return;
    }

    if (entry.key.startsWith(QLatin1String("x-")))
        return;

    if (parse_asset_entry_maybe(entry))
        return;


    if (m_cur_game)
        parse_game_entry(entry, sctx);
    else
        parse_collection_entry(entry);
}

} // namespace parser
} // namespace pegasus
} // namespace providers
