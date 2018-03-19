// Pegasus Frontend
// Copyright (C) 2017-2018  Mátyás Mustoha
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


#include "PegasusCollections.h"

#include "ConfigFile.h"
#include "PegasusCommon.h"
#include "types/Collection.h"
#include "types/Game.h"

#include <QDebug>
#include <QDirIterator>
#include <QRegularExpression>


namespace {

struct GameFilterGroup {
    QStringList extensions;
    QStringList files;
    QString regex;
};
struct GameFilter {
    GameFilterGroup include;
    GameFilterGroup exclude;
    QStringList extra;
};

void traverse_dir(const QString& dir_base_path,
                  const QHash<QString, GameFilter>& filter_config,
                  const QHash<QString, Types::Collection*>& collections,
                  QHash<QString, Types::Game*>& games)
{
    constexpr auto entry_filters = QDir::Files | QDir::Dirs | QDir::Readable | QDir::NoDotAndDotDot;
    constexpr auto entry_flags = QDirIterator::FollowSymlinks;

    auto config_it = filter_config.constBegin();
    for (; config_it != filter_config.constEnd(); ++config_it) {
        const GameFilter& filter = config_it.value();
        const QRegularExpression include_regex(filter.include.regex);
        const QRegularExpression exclude_regex(filter.exclude.regex);

        // find all dirs and subdirectories, but ignore 'media'

        QStringList subdirs;
        {
            constexpr auto subdir_filters = QDir::Dirs | QDir::Readable | QDir::NoDotAndDotDot;
            constexpr auto subdir_flags = QDirIterator::FollowSymlinks | QDirIterator::Subdirectories;

            QDirIterator dirs_it(dir_base_path, subdir_filters, subdir_flags);
            while (dirs_it.hasNext()) {
                subdirs << dirs_it.next();
            }
            subdirs.removeOne(dir_base_path + QStringLiteral("/media"));
            subdirs.append(dir_base_path + QStringLiteral("/")); // added "/" so all entries have base + 1 length
        }

        // run through the main dir and all valid subdirs

        for (const QString& dir : qAsConst(subdirs)) {
            QDirIterator dir_it(dir, entry_filters, entry_flags);
            while (dir_it.hasNext()) {
                dir_it.next();
                const QFileInfo fileinfo = dir_it.fileInfo();
                const QString relative_path = fileinfo.filePath().mid(dir_base_path.length() + 1);

                const bool exclude = filter.exclude.extensions.contains(fileinfo.suffix())
                    || filter.exclude.files.contains(relative_path)
                    || (!filter.exclude.regex.isEmpty() && exclude_regex.match(fileinfo.filePath()).hasMatch());
                if (exclude)
                    continue;

                const bool include = filter.include.extensions.contains(fileinfo.suffix())
                    || filter.include.files.contains(relative_path)
                    || (!filter.include.regex.isEmpty() && include_regex.match(fileinfo.filePath()).hasMatch());
                if (!include)
                    continue;

                Types::Collection* const& collection_ptr = collections[config_it.key()];
                Types::Game*& game_ptr = games[fileinfo.canonicalFilePath()];
                if (!game_ptr)
                    game_ptr = new Types::Game(fileinfo, collection_ptr);

                collection_ptr->gameListMut().addGame(game_ptr);
            }
        }
    }
}

} // namespace


namespace providers {
namespace pegasus {

enum class CollAttribType : unsigned char {
    LAUNCH_CMD,
    EXTENSIONS,
    FILES,
    REGEX,
};

QHash<QString, GameFilter> read_collections_file(const QHash<QString, CollAttribType>& key_types,
                                                 const QString& dir_path,
                                                 QHash<QString, Types::Collection*>& collections)
{
    // reminder: sections are collection tags
    // including keys: extensions, files, regex
    // excluding keys: ignore-extensions, ignore-files, ignore-regex
    // optional: name, launch

    QString curr_config_path;
    QHash<QString, GameFilter> config;
    Types::Collection* curr_coll = nullptr;

    const auto on_error = [&](const int lineno, const QString msg){
        qWarning().noquote()
            << QObject::tr("`%1`, line %2: %3")
                           .arg(curr_config_path, QString::number(lineno), msg);
    };
    const auto on_attribute = [&](const int lineno, const QString key, const QString val){
        if (key == QLatin1String("collection")) {
            curr_coll = nullptr;

            if (!collections.contains(val))
                collections.insert(val, new Types::Collection(val));

            curr_coll = collections[val];
            curr_coll->sourceDirsMut().append(dir_path);
            curr_coll->setName(val);
            return;
        }

        if (!curr_coll) {
            on_error(lineno, QObject::tr("no collection defined yet, entry ignored"));
            return;
        }

        if (key.startsWith(QLatin1String("x-"))) {
            // TODO: unimplemented
            return;
        }
        if (!key_types.contains(key)) {
            on_error(lineno, QObject::tr("unrecognized attribute name `%3`, ignored").arg(key));
            return;
        }

        GameFilter& filter = config[curr_coll->name()];
        GameFilterGroup& filter_group = key.startsWith(QLatin1String("ignore-"))
            ? filter.exclude
            : filter.include;
        switch (key_types[key]) {
            case CollAttribType::LAUNCH_CMD:
                curr_coll->setCommonLaunchCmd(val);
                break;
            case CollAttribType::EXTENSIONS:
                filter_group.extensions.append(tokenize(val.toLower()));
                break;
            case CollAttribType::FILES:
                filter_group.files.append(val);
                break;
            case CollAttribType::REGEX:
                filter_group.regex = val;
                break;
        }
    };


    // the actual reading

    curr_config_path = dir_path + QStringLiteral("/collections.pegasus.txt");
    config::readFile(curr_config_path,  on_attribute, on_error);

    curr_config_path = dir_path + QStringLiteral("/collections.txt");
    curr_coll = nullptr;
    config::readFile(curr_config_path, on_attribute, on_error);

    // cleanup and return

    auto config_it = config.begin();
    for (; config_it != config.end(); ++config_it) {
        GameFilter& filter = config_it.value();
        filter.include.extensions.removeDuplicates();
        filter.include.files.removeDuplicates();
        filter.exclude.extensions.removeDuplicates();
        filter.exclude.files.removeDuplicates();
        filter.extra.removeDuplicates();
    }
    return config;
}

PegasusCollections::PegasusCollections()
    : m_key_types {
        { QStringLiteral("launch"), CollAttribType::LAUNCH_CMD },
        { QStringLiteral("command"), CollAttribType::LAUNCH_CMD },
        { QStringLiteral("extension"), CollAttribType::EXTENSIONS },
        { QStringLiteral("extensions"), CollAttribType::EXTENSIONS },
        { QStringLiteral("file"), CollAttribType::FILES },
        { QStringLiteral("files"), CollAttribType::FILES },
        { QStringLiteral("regex"), CollAttribType::REGEX },
        { QStringLiteral("ignore-extension"), CollAttribType::EXTENSIONS },
        { QStringLiteral("ignore-extensions"), CollAttribType::EXTENSIONS },
        { QStringLiteral("ignore-file"), CollAttribType::FILES },
        { QStringLiteral("ignore-files"), CollAttribType::FILES },
        { QStringLiteral("ignore-regex"), CollAttribType::REGEX },
    }
{
}

void PegasusCollections::find_in_dirs(const QStringList& dir_list,
                                      QHash<QString, Types::Game*>& games,
                                      QHash<QString, Types::Collection*>& collections) const
{
    for (const QString& dir_path : dir_list) {
        const auto filter_config = read_collections_file(m_key_types, dir_path, collections);
        traverse_dir(dir_path, filter_config, collections, games);
    }
}

} // namespace pegasus
} // namespace providers
