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


#include "PegasusMetadataFilter.h"

#include "model/gaming/Collection.h"
#include "model/gaming/Game.h"
#include "utils/StdHelpers.h"
#include "providers/SearchContext.h"

#include <QDir>
#include <QDirIterator>


namespace {
using FileFilter = providers::pegasus::parser::FileFilter;

bool rx_match(const QRegularExpression& rx, const QString& str) {
    return !rx.pattern().isEmpty() && rx.match(str).hasMatch();
}

// Find all dirs and subdirectories, but ignore 'media'
std::vector<QString> all_valid_direct_subdirs(const QString& filter_dir)
{
    Q_ASSERT(!filter_dir.isEmpty());
    if (Q_UNLIKELY(filter_dir.isEmpty()))
        return {};

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

std::vector<QString> resolve_filelist(const std::vector<QString>& paths, const std::vector<QString>& dirs)
{
    std::vector<QString> can_paths;
    can_paths.reserve(paths.size() * dirs.size());

    for (const QString& dir : dirs) {
        for (const QString& path : paths)
            can_paths.emplace_back(QFileInfo(dir, path).canonicalFilePath());
    }

    VEC_REMOVE_IF(can_paths, [](const QString& s){ return s.isEmpty(); });
    VEC_REMOVE_DUPLICATES(can_paths);
    can_paths.shrink_to_fit();
    return can_paths;
}

bool file_passes_filter(const QFileInfo& finfo, const FileFilter& filter,
                        const std::vector<QString>& exclude_files)
{
    const QString file_ext = finfo.suffix().toLower();

    const bool exclude = VEC_CONTAINS(filter.exclude.extensions, file_ext)
        || VEC_CONTAINS(exclude_files, finfo.canonicalFilePath())
        || rx_match(filter.exclude.regex, finfo.filePath());
    if (exclude)
        return false;

    const bool include = VEC_CONTAINS(filter.include.extensions, file_ext)
        || rx_match(filter.include.regex, finfo.filePath());
    if (!include)
        return false;

    return true;
}

void accept_filtered_file(const QFileInfo& fileinfo,
                          providers::PendingCollection& parent,
                          providers::SearchContext& sctx)
{
    const providers::PendingGame& entry = sctx.add_or_create_game_from_file(fileinfo, parent);

    // When a game was defined earlier than its collection
    model::Game& game = entry.inner();
    if (game.launchCmd().isEmpty())
        game.setLaunchCmd(parent.inner().commonLaunchCmd());
    if (game.launchWorkdir().isEmpty())
        game.setLaunchWorkdir(parent.inner().commonLaunchWorkdir());
    if (game.launchCmdBasedir().isEmpty())
        game.setLaunchCmdBasedir(parent.inner().commonLaunchCmdBasedir());
}
} // namespace


namespace providers {
namespace pegasus {
namespace parser {

FileFilterGroup::FileFilterGroup() = default;

FileFilter::FileFilter(QString collection, QString base_dir)
    : collection_key(std::move(collection))
    , directories({std::move(base_dir)})
{
    Q_ASSERT(!collection_key.isEmpty());
    Q_ASSERT(!directories.front().isEmpty());
}


void tidy_filters(std::vector<FileFilter>& filters)
{
    for (FileFilter& filter : filters) {
        VEC_REMOVE_DUPLICATES(filter.directories);
        VEC_REMOVE_DUPLICATES(filter.include.extensions);
        VEC_REMOVE_DUPLICATES(filter.include.files);
        VEC_REMOVE_DUPLICATES(filter.exclude.extensions);
        VEC_REMOVE_DUPLICATES(filter.exclude.files);
    }
}

void process_filter(const FileFilter& filter, providers::SearchContext& sctx)
{
    PendingCollection& collection = sctx.get_or_create_collection(filter.collection_key);

    const std::vector<QString> include_files = resolve_filelist(filter.include.files, filter.directories);
    const std::vector<QString> exclude_files = resolve_filelist(filter.exclude.files, filter.directories);
    for (const QString& can_path: include_files) {
        if (!VEC_CONTAINS(exclude_files, can_path))
            accept_filtered_file(QFileInfo(can_path), collection, sctx);
    }

    const bool needs_scan = !filter.include.extensions.empty()
        || (!filter.include.regex.pattern().isEmpty() && filter.include.regex.isValid());
    if (!needs_scan)
        return;

    constexpr auto entry_filters_files = QDir::Files | QDir::Readable | QDir::NoDotAndDotDot;
    constexpr auto entry_filters_all = QDir::Dirs | entry_filters_files;
    constexpr auto entry_flags = QDirIterator::FollowSymlinks | QDirIterator::Subdirectories;
    for (const QString& filter_dir : filter.directories) {
        // directly contained files
        QDirIterator file_it(filter_dir, entry_filters_files);
        while (file_it.hasNext()) {
            file_it.next();
            if (file_passes_filter(file_it.fileInfo(), filter, exclude_files))
                accept_filtered_file(file_it.fileInfo(), collection, sctx);
        }

        // directly contained directories, except media
        const std::vector<QString> dirs_to_check = all_valid_direct_subdirs(filter_dir);
        for (const QString& subdir : dirs_to_check) {
            QDirIterator subdir_it(subdir, entry_filters_all, entry_flags);
            while (subdir_it.hasNext()) {
                subdir_it.next();
                if (file_passes_filter(subdir_it.fileInfo(), filter, exclude_files))
                    accept_filtered_file(subdir_it.fileInfo(), collection, sctx);
            }
        }
    }
}

void process_filters(const std::vector<FileFilter>& filters, providers::SearchContext& sctx)
{
    for (const FileFilter& filter : filters)
        process_filter(filter, sctx);
}

} // namespace parser
} // namespace pegasus
} // namespace providers
