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
#include "PegasusParser.h"
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

    if (!ctx.helpers.coll_attribs.count(entry.key)) {
        ctx.print_error(entry.line, tr_log("unrecognized collection property `%1`, ignored").arg(entry.key));
        return;
    }

    FileFilterGroup& filter_group = entry.key.startsWith(QLatin1String("ignore-"))
        ? ctx.cur_filter->exclude
        : ctx.cur_filter->include;

    switch (ctx.helpers.coll_attribs.at(entry.key)) {
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

void parse_game_entry(ParserContext& ctx, const config::Entry& entry)
{
    // NOTE: ctx.cur_coll may be null (ie. a game entry defined before any collection)
    Q_ASSERT(ctx.cur_game);

    if (!ctx.helpers.game_attribs.count(entry.key)) {
        ctx.print_error(entry.line, tr_log("unrecognized game property `%1`, ignored").arg(entry.key));
        return;
    }

    switch (ctx.helpers.game_attribs.at(entry.key)) {
        case GameAttrib::FILES:
            {
                auto& files = ctx.cur_game->files;
                for (const QString& line : entry.values)
                    files.emplace_back(QFileInfo(ctx.dir_path, line));

                utils::remove_dupli(files);
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
                const auto rx_match = ctx.helpers.rx_count_range.match(first_line_of(entry));
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
                const auto rx_match = ctx.helpers.rx_date.match(first_line_of(entry));
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

                const auto rx_match_a = ctx.helpers.rx_percent.match(line);
                if (rx_match_a.hasMatch()) {
                    ctx.cur_game->rating = qBound(0.f, line.leftRef(line.length() - 1).toFloat() / 100.f, 1.f);
                    return;
                }
                const auto rx_match_b = ctx.helpers.rx_float.match(line);
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

    const auto rx_match = ctx.helpers.rx_asset_key.match(entry.key);
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

void parse_entry(ParserContext& ctx, const config::Entry& entry)
{
    // TODO: set cur_* by the return value of emplace
    if (entry.key == QLatin1String("collection")) {
        const QString& name = first_line_of(entry);

        if (!ctx.outvars.collections.count(name))
            ctx.outvars.collections.emplace(name, modeldata::Collection(name));

        ctx.cur_coll = &ctx.outvars.collections.at(name);
        ctx.cur_game = nullptr;

        ctx.outvars.filters.emplace_back(name, ctx.dir_path);
        ctx.cur_filter = &ctx.outvars.filters.back();
        return;
    }

    if (entry.key == QLatin1String("game")) {
        modeldata::Game game(first_line_of(entry));
        if (ctx.cur_coll) {
            game.launch_cmd = ctx.cur_coll->launch_cmd;
            game.launch_workdir = ctx.cur_coll->launch_workdir;
        }
        ctx.outvars.games.emplace_back(std::move(game));
        ctx.cur_game = &ctx.outvars.games.back();
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
        parse_game_entry(ctx, entry);
    else
        parse_collection_entry(ctx, entry);
}

void read_metafile(const QString& metafile_path, OutputVars& output, const ParserHelpers& helpers)
{
    ParserContext ctx(metafile_path, output, helpers);

    const auto on_error = [&](const config::Error& error){
        ctx.print_error(error.line, error.message);
    };
    const auto on_entry = [&](const config::Entry& entry){
        parse_entry(ctx, entry);
    };

    if (!config::readFile(metafile_path, on_entry, on_error)) {
        qWarning().noquote() << MSG_PREFIX
            << tr_log("Failed to read metadata file %1, file ignored").arg(metafile_path);
        return;
    }
}

// collect collection and game information
void collect_metadata(const std::vector<QString>& dir_list, OutputVars& results)
{
    const ParserHelpers helpers;

    for (const QString& dir_path : dir_list) {
        const QString metafile = find_metafile_in(dir_path);
        if (metafile.isEmpty())
            continue;

        read_metafile(metafile, results, helpers);
    }
}

void remove_empty_games(std::vector<modeldata::Game>& games)
{
    auto it = std::remove_if(games.begin(), games.end(),
        [](const modeldata::Game& game) { return game.files.empty(); });

    for (auto printer_it = it; printer_it != games.end(); ++printer_it) {
        qWarning().noquote() << MSG_PREFIX
            << tr_log("No files defined for game '%1', ignored").arg(printer_it->title);
    }

    games.erase(it, games.end());
}

void build_path_map(const std::vector<modeldata::Game>& games,
                    HashMap<QString, size_t>& path_to_gameidx)
{
    for (size_t i = 0; i < games.size(); i++) {
        // empty games should have been removed already
        Q_ASSERT(games[i].files.size() > 0);

        for (const modeldata::GameFile& entry : games[i].files) {
            QString path = entry.fileinfo.canonicalFilePath();

            // File s are added to the game only if they exist;
            // the canonical path will be empty only if the file was deleted since this check
            Q_ASSERT(!path.isEmpty());
            if (Q_LIKELY(!path.isEmpty()))
                path_to_gameidx.emplace(std::move(path), i);
        }
    }
}

void tidy_filters(std::vector<FileFilter>& filters)
{
    for (FileFilter& filter : filters) {
        filter.directories.removeDuplicates();
        filter.include.extensions.removeDuplicates();
        filter.include.files.removeDuplicates();
        filter.exclude.extensions.removeDuplicates();
        filter.exclude.files.removeDuplicates();
    }
}

// Find all dirs and subdirectories, but ignore 'media'
QVector<QString> filter_find_dirs(const QString& filter_dir)
{
    constexpr auto subdir_filters = QDir::Dirs | QDir::Readable | QDir::NoDotAndDotDot;
    constexpr auto subdir_flags = QDirIterator::FollowSymlinks | QDirIterator::Subdirectories;

    QVector<QString> result;

    QDirIterator dirs_it(filter_dir, subdir_filters, subdir_flags);
    while (dirs_it.hasNext())
        result << dirs_it.next();

    result.removeOne(filter_dir + QStringLiteral("/media"));
    result.append(filter_dir + QStringLiteral("/")); // added "/" so all entries have base + 1 length

    return result;
}

bool file_passes_filter(const QFileInfo& fileinfo, const FileFilter& filter, const QString filter_dir)
{
    const QString relative_path = fileinfo.filePath().mid(filter_dir.length() + 1);

    const bool exclude = filter.exclude.extensions.contains(fileinfo.suffix())
        || filter.exclude.files.contains(relative_path)
        || (!filter.exclude.regex.pattern().isEmpty() && filter.exclude.regex.match(fileinfo.filePath()).hasMatch());
    if (exclude)
        return false;

    const bool include = filter.include.extensions.contains(fileinfo.suffix())
        || filter.include.files.contains(relative_path)
        || (!filter.include.regex.pattern().isEmpty() && filter.include.regex.match(fileinfo.filePath()).hasMatch());
    if (!include)
        return false;

    return true;
}

void accept_filtered_file(const QFileInfo& fileinfo, const modeldata::Collection& parent,
                          providers::SearchContext& sctx)
{
    const QString game_path = fileinfo.canonicalFilePath();
    if (!sctx.path_to_gameidx.count(game_path)) {
        // This means there weren't any game entries with matching file entry
        // in any of the parsed metadata files. There is no existing game data
        // created yet either.
        modeldata::Game game(fileinfo);
        game.launch_cmd = parent.launch_cmd;
        game.launch_workdir = parent.launch_workdir;

        sctx.path_to_gameidx.emplace(game_path, sctx.games.size());
        sctx.games.emplace_back(std::move(game));
    }
    const size_t game_idx = sctx.path_to_gameidx.at(game_path);
    sctx.collection_childs[parent.name].emplace_back(game_idx);

    // When a game was defined earlier than its collection
    modeldata::Game& game = sctx.games.at(game_idx);
    if (game.launch_cmd.isEmpty())
        game.launch_cmd = parent.launch_cmd;
    if (game.launch_workdir.isEmpty())
        game.launch_workdir = parent.launch_workdir;
}

void process_filters(const std::vector<FileFilter>& filters, providers::SearchContext& sctx)
{
    constexpr auto entry_filters = QDir::Files | QDir::Dirs | QDir::Readable | QDir::NoDotAndDotDot;
    constexpr auto entry_flags = QDirIterator::FollowSymlinks;

    for (const FileFilter& filter : filters) {
        const modeldata::Collection& collection = sctx.collections.at(filter.collection_name);

        for (const QString& filter_dir : filter.directories) {
            // ie. all dirs and subdirs except /media
            const QVector<QString> dirs_to_check = filter_find_dirs(filter_dir);

            for (const QString& subdir : dirs_to_check) {
                QDirIterator subdir_it(subdir, entry_filters, entry_flags);
                while (subdir_it.hasNext()) {
                    subdir_it.next();
                    const QFileInfo fileinfo = subdir_it.fileInfo();

                    if (file_passes_filter(fileinfo, filter, filter_dir))
                        accept_filtered_file(fileinfo, collection, sctx);
                }
            }
        }

        const auto coll_childs_it = sctx.collection_childs.find(collection.name);
        if (coll_childs_it != sctx.collection_childs.cend()) {
            auto& vec = coll_childs_it->second;
            std::sort(vec.begin(), vec.end());
            vec.erase(std::unique(vec.begin(), vec.end()), vec.end());
        }
    }
}

} // namespace


namespace providers {
namespace pegasus {

void find_in_dirs(const std::vector<QString>& dir_list, providers::SearchContext& sctx)
{
    OutputVars results(sctx.collections, sctx.games);
    collect_metadata(dir_list, results);

    remove_empty_games(sctx.games);
    build_path_map(sctx.games, sctx.path_to_gameidx);

    tidy_filters(results.filters);
    process_filters(results.filters, sctx);
}

} // namespace pegasus
} // namespace providers
