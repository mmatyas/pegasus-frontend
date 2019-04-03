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

#include "LocaleUtils.h"
#include "Paths.h"
#include "PegasusMetadataConstants.h"
#include "PegasusMetadataFilter.h"
#include "PegasusMetadataParser.h"
#include "parsers/MetaFile.h"
#include "providers/Provider.h"
#include "utils/StdHelpers.h"

#include <QDebug>
#include <QDir>
#include <QDirIterator>

using namespace providers::pegasus::parser;


namespace {
static constexpr auto MSG_PREFIX = "Metafiles:";


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

std::vector<QString> find_global_metafiles()
{
    constexpr auto dir_filters = QDir::Files | QDir::Readable | QDir::NoDotAndDotDot;
    constexpr auto dir_flags = QDirIterator::FollowSymlinks;
    const QRegularExpression rx_path(QStringLiteral("^(.+\\.)?metadata(\\.pegasus)?\\.txt$"));

    std::vector<QString> result;

    const QString dir_path = paths::writableConfigDir() + QLatin1String("/metafiles");
    QDirIterator dir_it(dir_path, dir_filters, dir_flags);
    while (dir_it.hasNext()) {
        QString path = dir_it.next();
        if (rx_path.match(dir_it.fileName()).hasMatch()) {
            qInfo().noquote() << MSG_PREFIX << tr_log("found `%1`").arg(path);
            result.emplace_back(std::move(path));
        }
    }

    return result;
}

void read_metafile(const QString& metafile_path,
                   providers::SearchContext& sctx,
                   std::vector<FileFilter>& filters,
                   const Constants& constants)
{
    Parser parser(metafile_path, constants);

    const auto on_error = [&](const metafile::Error& error){
        parser.print_error(error.line, error.message);
    };
    const auto on_entry = [&](const metafile::Entry& entry){
        parser.parse_entry(entry, sctx, filters);
    };

    if (!metafile::read_file(metafile_path, on_entry, on_error)) {
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

    for (const QString& metafile : find_global_metafiles())
        read_metafile(metafile, sctx, filters, constants);

    for (const QString& dir_path : dir_list) {
        const QString metafile = find_metafile_in(dir_path);
        if (!metafile.isEmpty()) {
            sctx.game_root_dirs.emplace_back(dir_path);
            read_metafile(metafile, sctx, filters, constants);
        }
    }
}

void move_collection_dirs_to(std::vector<QString>& out, std::vector<FileFilter>& filters)
{
    size_t count = out.size();
    for (const FileFilter& filter : filters)
        count += filter.directories.size();

    out.reserve(count);
    for (FileFilter& filter : filters)
        vec_append_move(out, filter.directories);
}
} // namespace


namespace providers {
namespace pegasus {

void find_in_dirs(std::vector<QString>& dir_list, providers::SearchContext& sctx)
{
    std::vector<FileFilter> filters;
    filters.reserve(dir_list.size());

    collect_metadata(dir_list, sctx, filters);

    tidy_filters(filters);
    process_filters(filters, sctx);

    move_collection_dirs_to(sctx.game_root_dirs, filters);
}

} // namespace pegasus
} // namespace providers
