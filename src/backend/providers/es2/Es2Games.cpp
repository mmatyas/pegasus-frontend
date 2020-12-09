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


#include "Es2Games.h"

#include "Log.h"
#include "Paths.h"
#include "model/gaming/Collection.h"
#include "providers/SearchContext.h"
#include "providers/es2/Es2Systems.h"
#include "utils/StdHelpers.h"

#include <QDirIterator>
#include <QFile>
#include <QRegularExpression>
#include <QStringBuilder>
#include <QTextStream>


namespace {
QVector<QStringRef> split_list(const QString& str)
{
    // FIXME: don't leave statics around
    static const QRegularExpression separator(QStringLiteral("[,\\s]"));
    return str.splitRef(separator, Qt::SkipEmptyParts);
}

/// returns a list of unique, '*.'-prefixed lowercase file extensions
QStringList parse_filters(const QString& filters_raw) {
    const QString filters_lowercase = filters_raw.toLower();
    const QVector<QStringRef> filter_refs = split_list(filters_lowercase);

    QStringList filter_list;
    for (const QStringRef& filter_ref : filter_refs)
        filter_list.append(QChar('*') + filter_ref.trimmed());

    filter_list.removeDuplicates();
    return filter_list;
}
} // namespace


namespace providers {
namespace es2 {

std::vector<QString> read_mame_blacklists(const QString& log_tag, const std::vector<QString>& possible_config_dirs)
{
    using L1Str = QLatin1String;

    const QString resources_path = possible_config_dirs.front() % L1Str("/resources/");
    const std::vector<std::pair<L1Str, L1Str>> blacklists {
        { L1Str("mamebioses.xml"), L1Str("bios") },
        { L1Str("mamedevices.xml"), L1Str("device") },
    };

    std::vector<QString> out;

    // TODO: C++17
    for (const auto& blacklist_entry : blacklists) {
        const QString file_path = resources_path % blacklist_entry.first;
        QFile file(file_path);
        if (!file.open(QFile::ReadOnly | QFile::Text))
            continue;

        const QString line_head = QStringLiteral("<%1>").arg(blacklist_entry.second);
        const QString line_tail = QStringLiteral("</%1>").arg(blacklist_entry.second);

        QTextStream stream(&file);
        QString line;
        int hit_count = 0;
        while (stream.readLineInto(&line, 128)) {
            const bool is_valid = line.startsWith(line_head) && line.endsWith(line_tail);
            if (!is_valid)
                continue;

            const int len = line.length() - line_head.length() - line_tail.length();
            out.emplace_back(line.mid(line_head.length(), len));
            hit_count++;
        }

        Log::info(log_tag, LOGMSG("Found `%1`, %2 entries loaded").arg(file_path, QString::number(hit_count)));
    }

    return out;
}

size_t find_games_for(
    const SystemEntry& sysentry,
    SearchContext& sctx,
    const std::vector<QString>& filename_blacklist)
{
    model::Collection& collection = *sctx.get_or_create_collection(sysentry.name);
    collection
        .setShortName(sysentry.shortname)
        .setCommonLaunchCmd(sysentry.launch_cmd);

    // find all (sub-)directories, but ignore 'media'
    const QStringList dirs = [&sysentry]{
        QStringList result;

        constexpr auto subdir_filters = QDir::Dirs | QDir::Readable | QDir::NoDotAndDotDot;
        constexpr auto subdir_flags = QDirIterator::FollowSymlinks | QDirIterator::Subdirectories;
        QDirIterator dirs_it(sysentry.path, subdir_filters, subdir_flags);
        while (dirs_it.hasNext())
            result.append(dirs_it.next());

        result.removeOne(sysentry.path + QStringLiteral("/media"));
        result.append(sysentry.path);
        return result;
    }();

    // use the blacklist maybe
    const QVector<QStringRef> platforms = split_list(sysentry.platforms);
    const bool use_blacklist = VEC_CONTAINS(platforms, QLatin1String("arcade"))
                            || VEC_CONTAINS(platforms, QLatin1String("neogeo"));


    // scan for game files

    constexpr auto entry_filters = QDir::Files | QDir::Dirs | QDir::Readable | QDir::NoDotAndDotDot;
    constexpr auto entry_flags = QDirIterator::FollowSymlinks;
    const QStringList name_filters = parse_filters(sysentry.extensions);

    size_t found_games = 0;
    for (const QString& dir_path : dirs) {
        QDirIterator files_it(dir_path, name_filters, entry_filters, entry_flags);
        while (files_it.hasNext()) {
            files_it.next();
            QFileInfo fileinfo = files_it.fileInfo();

            const QString filename = fileinfo.completeBaseName();
            if (use_blacklist && VEC_CONTAINS(filename_blacklist, filename))
                continue;

            model::Game* game_ptr = sctx.game_by_filepath(fileinfo.canonicalFilePath());
            if (!game_ptr) {
                game_ptr = sctx.create_game_for(collection);
                sctx.game_add_filepath(*game_ptr, fileinfo.canonicalFilePath());
            }
            sctx.game_add_to(*game_ptr, collection);
            found_games++;
        }
    }

    return found_games;
}

} // namespace es2
} // namespace providers
