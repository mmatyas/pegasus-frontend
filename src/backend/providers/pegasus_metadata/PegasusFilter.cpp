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


#include "PegasusFilter.h"

#include "model/gaming/Collection.h"
#include "model/gaming/Game.h"
#include "providers/SearchContext.h"
#include "utils/PathTools.h"
#include "utils/StdHelpers.h"

#include <QDirIterator>


namespace {
// Find all dirs and subdirectories, but ignore 'media'
std::vector<QString> all_valid_direct_subdirs(const QString& filter_dir)
{
    Q_ASSERT(!filter_dir.isEmpty());

    constexpr auto subdir_filters = QDir::Dirs | QDir::Readable | QDir::NoDotAndDotDot;
    constexpr auto subdir_flags = QDirIterator::FollowSymlinks;

    std::vector<QString> result;

    QDirIterator dirs_it(filter_dir, subdir_filters, subdir_flags);
    while (dirs_it.hasNext())
        result.emplace_back(dirs_it.next());

    const QString media_dir = filter_dir + QStringLiteral("/media");
    VEC_REMOVE_VALUE(result, media_dir);

    return result;
}

std::vector<QString> resolve_filelist(const std::vector<QString>& relpaths, const std::vector<QString>& dirs)
{
    std::vector<QString> found_paths;
    found_paths.reserve(relpaths.size() * dirs.size());

    for (const QString& dir_str : dirs) {
        const QDir dir(dir_str);
        for (const QString& relpath : relpaths) {
            QString path = ::clean_abs_path(QFileInfo(dir, relpath));
            if (!path.isEmpty())
                found_paths.emplace_back(std::move(path));
        }
    }

    VEC_REMOVE_DUPLICATES(found_paths);
    found_paths.shrink_to_fit();
    return found_paths;
}

bool rx_match(const QRegularExpression& rx, const QString& str) {
    return !rx.pattern().isEmpty() && rx.match(str).hasMatch();
}

bool file_passes_filter(
    const QFileInfo& finfo,
    const providers::pegasus::FileFilter& filter,
    const std::vector<QString>& exclude_files)
{
    const QString file_ext = finfo.suffix().toLower();

    const bool exclude = VEC_CONTAINS(filter.exclude.extensions, file_ext)
        || VEC_CONTAINS(exclude_files, ::clean_abs_path(finfo))
        || rx_match(filter.exclude.regex, finfo.filePath());
    if (exclude)
        return false;

    const bool include = VEC_CONTAINS(filter.include.extensions, file_ext)
        || rx_match(filter.include.regex, finfo.filePath());
    if (!include)
        return false;

    return true;
}

void accept_filtered_file(
    const QString& filepath,
    model::Collection& collection,
    providers::SearchContext& sctx)
{
    model::Game* game_ptr = sctx.game_by_filepath(filepath);
    if (!game_ptr) {
        game_ptr = sctx.create_game_for(collection);
        sctx.game_add_filepath(*game_ptr, filepath);
    }

    model::Game& game = *game_ptr;
    sctx.game_add_to(game, collection);
}
} // namespace


namespace providers {
namespace pegasus {

FileFilterGroup::FileFilterGroup() = default;

FileFilter::FileFilter(model::Collection* const collection_ptr, QString base_dir)
    : collection(collection_ptr)
    , directories({ std::move(base_dir) })
{
    Q_ASSERT(collection);
    Q_ASSERT(!directories.front().isEmpty());
}

void apply_filter(FileFilter& filter, SearchContext& sctx)
{
    VEC_REMOVE_DUPLICATES(filter.directories);
    VEC_REMOVE_DUPLICATES(filter.include.extensions);
    VEC_REMOVE_DUPLICATES(filter.include.files);
    VEC_REMOVE_DUPLICATES(filter.exclude.extensions);
    VEC_REMOVE_DUPLICATES(filter.exclude.files);

    Q_ASSERT(filter.collection);
    model::Collection& collection = *filter.collection;

    const std::vector<QString> include_files = resolve_filelist(filter.include.files, filter.directories);
    const std::vector<QString> exclude_files = resolve_filelist(filter.exclude.files, filter.directories);
    for (const QString& filepath: include_files) {
        if (VEC_CONTAINS(exclude_files, filepath))
            continue;
        if (QFileInfo::exists(filepath))
            accept_filtered_file(filepath, collection, sctx);
    }

    const bool has_valid_regex = !filter.include.regex.pattern().isEmpty() && filter.include.regex.isValid();
    const bool needs_scan = !filter.include.extensions.empty() || has_valid_regex;
    if (!needs_scan)
        return;

    constexpr auto entry_filters_files = QDir::Files | QDir::Readable | QDir::NoDotAndDotDot;
    constexpr auto entry_filters_all = QDir::Dirs | entry_filters_files;
    constexpr auto entry_flags = QDirIterator::FollowSymlinks | QDirIterator::Subdirectories;
    for (const QString& filter_dir : filter.directories) {
        Q_ASSERT(!filter_dir.isEmpty());

        // directly contained files
        QDirIterator file_it(filter_dir, entry_filters_files);
        while (file_it.hasNext()) {
            file_it.next();
            const QString path = ::clean_abs_path(file_it.fileInfo());
            if (file_passes_filter(file_it.fileInfo(), filter, exclude_files))
                accept_filtered_file(path, collection, sctx);
        }

        // directly contained directories, except media
        const std::vector<QString> dirs_to_check = all_valid_direct_subdirs(filter_dir);
        for (const QString& subdir : dirs_to_check) {
            QDirIterator subdir_it(subdir, entry_filters_all, entry_flags);
            while (subdir_it.hasNext()) {
                subdir_it.next();
                const QString path = ::clean_abs_path(subdir_it.fileInfo());
                if (file_passes_filter(subdir_it.fileInfo(), filter, exclude_files))
                    accept_filtered_file(path, collection, sctx);
            }
        }
    }
}

} // namespace pegasus
} // namespace providers
