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


#include "PegasusMetadata.h"

#include "ConfigFile.h"
#include "LocaleUtils.h"
#include "Paths.h"
#include "PegasusAssets.h"
#include "PegasusMetadataFilter.h"
#include "PegasusMetadataContext.h"
#include "PegasusUtils.h"
#include "modeldata/gaming/CollectionData.h"
#include "modeldata/gaming/GameData.h"
#include "utils/PathCheck.h"
#include "utils/StdHelpers.h"

#include <QDebug>
#include <QDirIterator>
#include <QRegularExpression>
#include <QStringBuilder>


namespace {
using namespace providers::pegasus::filter;
using namespace providers::pegasus::parser;
using namespace providers::pegasus::utils;

static constexpr auto MSG_PREFIX = "Collections:";


const QString& first_line_of(const config::Entry& entry)
{
    Q_ASSERT(!entry.key.isEmpty());
    Q_ASSERT(!entry.values.isEmpty());

    if (entry.values.count() > 1) {
        qWarning().noquote() << MSG_PREFIX
            << tr_log("Expected single line value for `%1` but got more. The rest of the lines will be ignored.")
               .arg(entry.key);
    }

    return entry.values.first();
}

QString find_metafile_in(const QString& dir_path)
{
    Q_ASSERT(!dir_path.isEmpty());

    // TODO: move metadata first after some transition period
    const QString possible_paths[] {
        dir_path + QStringLiteral("/collections.pegasus.txt"),
        dir_path + QStringLiteral("/metadata.pegasus.txt"),
        dir_path + QStringLiteral("/collections.txt"),
        dir_path + QStringLiteral("/metadata.txt"),
    };

    for (const QString& path : possible_paths) {
        if (QFileInfo::exists(path)) {
            qInfo().noquote() << MSG_PREFIX
                << tr_log("found `%1`").arg(path);
            return path;
        }
    }

    qWarning().noquote() << MSG_PREFIX
        << tr_log("No metadata file found in `%1`, directory ignored").arg(dir_path);
    return QString();
}

void parse_collection_entry(ParserContext& ctx, const config::Entry& entry)
{
    Q_ASSERT(ctx.cur_coll);
    Q_ASSERT(ctx.cur_filter);
    Q_ASSERT(!ctx.cur_game);

    if (!ctx.constants.coll_attribs.count(entry.key)) {
        ctx.print_error(entry.line, tr_log("unrecognized collection property `%1`, ignored").arg(entry.key));
        return;
    }

    FileFilterGroup& filter_group = entry.key.startsWith(QLatin1String("ignore-"))
        ? ctx.cur_filter->exclude
        : ctx.cur_filter->include;

    switch (ctx.constants.coll_attribs.at(entry.key)) {
        case CollAttrib::SHORT_NAME:
            ctx.cur_coll->setShortName(first_line_of(entry));
            break;
        case CollAttrib::LAUNCH_CMD:
            ctx.cur_coll->launch_cmd = config::mergeLines(entry.values);
            break;
        case CollAttrib::LAUNCH_WORKDIR:
            ctx.cur_coll->launch_workdir = first_line_of(entry);
            break;
        case CollAttrib::DIRECTORIES:
            for (const QString& value : entry.values) {
                QFileInfo finfo(value);
                if (finfo.isRelative())
                    finfo.setFile(ctx.dir_path % '/' % value);

                ctx.cur_filter->directories.append(finfo.canonicalFilePath());
            }
            break;
        case CollAttrib::EXTENSIONS:
            filter_group.extensions.append(tokenize_by_comma(first_line_of(entry).toLower()));
            break;
        case CollAttrib::FILES:
            for (const QString& value : entry.values)
                filter_group.files.append(value);
            break;
        case CollAttrib::REGEX:
            filter_group.regex.setPattern(first_line_of(entry));
            if (!filter_group.regex.isValid()) {
                ctx.print_error(entry.line, tr_log("invalid regular expression: %1")
                    .arg(filter_group.regex.errorString()));
            }
            break;
        case CollAttrib::SHORT_DESC:
            ctx.cur_coll->summary = config::mergeLines(entry.values);
            break;
        case CollAttrib::LONG_DESC:
            ctx.cur_coll->description = config::mergeLines(entry.values);
            break;
    }
}

void parse_game_entry(ParserContext& ctx, providers::SearchContext& sctx, const config::Entry& entry)
{
    // NOTE: ctx.cur_coll may be null (ie. a game entry defined before any collection)
    Q_ASSERT(ctx.cur_game);

    if (!ctx.constants.game_attribs.count(entry.key)) {
        ctx.print_error(entry.line, tr_log("unrecognized game property `%1`, ignored").arg(entry.key));
        return;
    }

    switch (ctx.constants.game_attribs.at(entry.key)) {
        case GameAttrib::FILES:
            {
                const size_t game_id = sctx.games.size() - 1; // FIXME: improve this

                auto& files = ctx.cur_game->files;
                for (const QString& line : entry.values) {
                    QFileInfo fi(ctx.dir_path, line);

                    QString path = fi.canonicalFilePath();
                    if (path.isEmpty()) {
                        ctx.print_error(entry.line, tr_log("missing file `%1`").arg(line));
                        continue;
                    }
                    if (sctx.path_to_gameid.count(path) && sctx.path_to_gameid.at(path) == game_id) {
                        ctx.print_error(entry.line, tr_log("duplicate file `%1`").arg(line));
                        continue;
                    }

                    // NOTE: the case when a file is set for multiple games
                    // is not handled at the moment
                    sctx.path_to_gameid.emplace(std::move(path), game_id);
                    files.emplace_back(std::move(fi));
                }
            }
            break;
        case GameAttrib::DEVELOPERS:
            for (const QString& line : entry.values)
                ctx.cur_game->developers.append(line);
            break;
        case GameAttrib::PUBLISHERS:
            for (const QString& line : entry.values)
                ctx.cur_game->publishers.append(line);
            break;
        case GameAttrib::GENRES:
            for (const QString& line : entry.values)
                ctx.cur_game->genres.append(line);
            break;
        case GameAttrib::PLAYER_COUNT:
            {
                const auto rx_match = ctx.constants.rx_count_range.match(first_line_of(entry));
                if (rx_match.hasMatch()) {
                    const short a = rx_match.capturedRef(1).toShort();
                    const short b = rx_match.capturedRef(3).toShort();
                    ctx.cur_game->player_count = std::max({static_cast<short>(1), a, b});
                }
            }
            break;
        case GameAttrib::SHORT_DESC:
            ctx.cur_game->summary = config::mergeLines(entry.values);
            break;
        case GameAttrib::LONG_DESC:
            ctx.cur_game->description = config::mergeLines(entry.values);
            break;
        case GameAttrib::RELEASE:
            {
                const auto rx_match = ctx.constants.rx_date.match(first_line_of(entry));
                if (!rx_match.hasMatch()) {
                    ctx.print_error(entry.line, tr_log("incorrect date format, should be YYYY, YYYY-MM or YYYY-MM-DD"));
                    return;
                }

                const int y = qMax(1, rx_match.captured(1).toInt());
                const int m = qBound(1, rx_match.captured(3).toInt(), 12);
                const int d = qBound(1, rx_match.captured(5).toInt(), 31);
                ctx.cur_game->release_date = QDate(y, m, d);
            }
            break;
        case GameAttrib::RATING:
            {
                const QString& line = first_line_of(entry);

                const auto rx_match_a = ctx.constants.rx_percent.match(line);
                if (rx_match_a.hasMatch()) {
                    ctx.cur_game->rating = qBound(0.f, line.leftRef(line.length() - 1).toFloat() / 100.f, 1.f);
                    return;
                }
                const auto rx_match_b = ctx.constants.rx_float.match(line);
                if (rx_match_b.hasMatch()) {
                    ctx.cur_game->rating = qBound(0.f, line.toFloat(), 1.f);
                    return;
                }

                ctx.print_error(entry.line, tr_log("failed to parse rating value"));
            }
            break;
        case GameAttrib::LAUNCH_CMD:
            ctx.cur_game->launch_cmd = first_line_of(entry);
            break;
        case GameAttrib::LAUNCH_WORKDIR:
            ctx.cur_game->launch_workdir = first_line_of(entry);
            break;
    }
}

// Returns true if the entry key matches asset regex.
// The actual asset type check may still fail however.
bool parse_asset_entry_maybe(ParserContext& ctx, const config::Entry& entry)
{
    Q_ASSERT(ctx.cur_coll || ctx.cur_game);

    const auto rx_match = ctx.constants.rx_asset_key.match(entry.key);
    if (!rx_match.hasMatch())
        return false;

    const QString asset_key = rx_match.captured(1);
    const AssetType asset_type = pegasus_assets::str_to_type(asset_key);
    if (asset_type == AssetType::UNKNOWN) {
        ctx.print_error(entry.line, tr_log("unknown asset type '%1', entry ignored").arg(asset_key));
        return true;
    }

    modeldata::GameAssets& assets = ctx.cur_game
        ? ctx.cur_game->assets
        : ctx.cur_coll->assets;
    assets.addUrlMaybe(asset_type, assetline_to_url(entry.values.first(), ctx.dir_path));
    return true;
}

void parse_entry(const config::Entry& entry,
                 ParserContext& ctx,
                 providers::SearchContext& sctx,
                 std::vector<FileFilter>& filters)
{
    // TODO: set cur_* by the return value of emplace
    if (entry.key == QLatin1String("collection")) {
        const QString& name = first_line_of(entry);

        if (!sctx.collections.count(name))
            sctx.collections.emplace(name, modeldata::Collection(name));

        ctx.cur_coll = &sctx.collections.at(name);
        ctx.cur_game = nullptr;

        filters.emplace_back(name, ctx.dir_path);
        ctx.cur_filter = &filters.back();
        return;
    }

    if (entry.key == QLatin1String("game")) {
        modeldata::Game game(first_line_of(entry));
        if (ctx.cur_coll) {
            game.launch_cmd = ctx.cur_coll->launch_cmd;
            game.launch_workdir = ctx.cur_coll->launch_workdir;
        }
        const size_t game_id = sctx.games.size();
        sctx.games.emplace(game_id, std::move(game));
        ctx.cur_game = &sctx.games.at(game_id);
        return;
    }


    if (!ctx.cur_coll && !ctx.cur_game) {
        ctx.print_error(entry.line, tr_log("no `collection` or `game` defined yet, entry ignored"));
        return;
    }

    if (entry.key.startsWith(QLatin1String("x-")))
        return;

    if (parse_asset_entry_maybe(ctx, entry))
        return;


    if (ctx.cur_game)
        parse_game_entry(ctx, sctx, entry);
    else
        parse_collection_entry(ctx, entry);
}

void read_metafile(const QString& metafile_path,
                   providers::SearchContext& sctx,
                   std::vector<FileFilter>& filters,
                   const Constants& constants)
{
    ParserContext ctx(metafile_path, constants);

    const auto on_error = [&](const config::Error& error){
        ctx.print_error(error.line, error.message);
    };
    const auto on_entry = [&](const config::Entry& entry){
        parse_entry(entry, ctx, sctx, filters);
    };

    if (!config::readFile(metafile_path, on_entry, on_error)) {
        qWarning().noquote() << MSG_PREFIX
            << tr_log("Failed to read metadata file %1, file ignored").arg(metafile_path);
        return;
    }
}

void collect_metadata(const std::vector<QString>& dir_list,
                      providers::SearchContext& sctx,
                      std::vector<FileFilter>& filters)
{
    const Constants constants;

    for (const QString& dir_path : dir_list) {
        const QString metafile = find_metafile_in(dir_path);
        if (metafile.isEmpty())
            continue;

        read_metafile(metafile, sctx, filters, constants);
    }
}

} // namespace


namespace providers {
namespace pegasus {

void find_in_dirs(const std::vector<QString>& dir_list, providers::SearchContext& sctx)
{
    std::vector<FileFilter> filters;
    filters.reserve(dir_list.size());

    collect_metadata(dir_list, sctx, filters);

    tidy_filters(filters);
    process_filters(filters, sctx);
}

} // namespace pegasus
} // namespace providers
