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


#include "PegasusProvider.h"

#include "Log.h"
#include "Paths.h"
#include "providers/SearchContext.h"
#include "providers/pegasus_metadata/PegasusMetadata.h"
#include "providers/pegasus_metadata/PegasusFilter.h"
#include "utils/PathTools.h"
#include "utils/StdHelpers.h"

#include <QDirIterator>


namespace {
bool is_metadata_file(const QString& filename)
{
    return filename == QLatin1String("metadata.pegasus.txt")
        || filename == QLatin1String("metadata.txt")
        || filename.endsWith(QLatin1String(".metadata.pegasus.txt"))
        || filename.endsWith(QLatin1String(".metadata.txt"));
}

std::vector<QString> find_metafiles_in(const QString& dir_path)
{
    constexpr auto dir_filters = QDir::Files | QDir::Readable | QDir::NoDotAndDotDot;
    constexpr auto dir_flags = QDirIterator::FollowSymlinks;

    std::vector<QString> result;

    QDirIterator dir_it(dir_path, dir_filters, dir_flags);
    while (dir_it.hasNext()) {
        dir_it.next();
        QString path = ::clean_abs_path(dir_it.fileInfo());
        if (is_metadata_file(dir_it.fileName()))
            result.emplace_back(std::move(path));
    }

    return result;
}

std::vector<QString> find_all_metafiles(const QStringList& gamedirs)
{
    const QString global_metafile_dir = paths::writableConfigDir() + QLatin1String("/metafiles");
    std::vector<QString> result = find_metafiles_in(global_metafile_dir);

    result.reserve(result.size() + gamedirs.size());
    for (const QString& dir_path : gamedirs) {
        std::vector<QString> local_metafiles = find_metafiles_in(dir_path);
        result.insert(result.end(),
            std::make_move_iterator(local_metafiles.begin()),
            std::make_move_iterator(local_metafiles.end()));
    }

    VEC_REMOVE_DUPLICATES(result);
    return result;
}
} // namespace


namespace providers {
namespace pegasus {

PegasusProvider::PegasusProvider(QObject* parent)
    : Provider(QLatin1String("pegasus_metafiles"), QStringLiteral("Metafiles"), PROVIDER_FLAG_INTERNAL, parent)
{}

Provider& PegasusProvider::run(SearchContext& sctx)
{
    const std::vector<QString> metafile_paths = find_all_metafiles(sctx.root_game_dirs());
    if (metafile_paths.empty()) {
        Log::info(display_name(), LOGMSG("No metadata files found"));
        return *this;
    }

    const Metadata metahelper(display_name());
    std::vector<FileFilter> all_filters;

    const float progress_step = 1.f / metafile_paths.size();
    float progress = 0.f;

    for (const QString& path : metafile_paths) {
        Log::info(display_name(), LOGMSG("Found `%1`").arg(::pretty_path(path)));

        std::vector<FileFilter> filters = metahelper.apply_metafile(path, sctx);
        all_filters.insert(all_filters.end(),
            std::make_move_iterator(filters.begin()),
            std::make_move_iterator(filters.end()));

        progress += progress_step;
        emit progressChanged(progress);
    }

    for (FileFilter& filter : all_filters) {
        apply_filter(filter, sctx);

        for (QString& dir_path : filter.directories)
            sctx.pegasus_add_game_dir(dir_path);
    }

    return *this;
}

} // namespace pegasus
} // namespace providers
