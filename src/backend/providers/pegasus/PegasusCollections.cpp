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
#include "LocaleUtils.h"
#include "PegasusCommon.h"
#include "Utils.h"
#include "modeldata/gaming/Collection.h"
#include "modeldata/gaming/Game.h"

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
                  const HashMap<QString, GameFilter>& filter_config,
                  HashMap<QString, modeldata::Game>& games,
                  HashMap<QString, modeldata::Collection>& collections,
                  HashMap<QString, std::vector<QString>>& collection_childs)
{
    constexpr auto entry_filters = QDir::Files | QDir::Dirs | QDir::Readable | QDir::NoDotAndDotDot;
    constexpr auto entry_flags = QDirIterator::FollowSymlinks;

    for (auto& config_it : filter_config) {
        const GameFilter& filter = config_it.second;
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
                QFileInfo fileinfo = dir_it.fileInfo();
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


                const QString coll_key = config_it.first;
                const QString game_key = fileinfo.canonicalFilePath();
                if (!games.count(game_key)) {
                    modeldata::Game game(std::move(fileinfo));
                    game.launch_cmd = collections.at(coll_key).launchCmd();
                    games.emplace(game_key, std::move(game));
                }
                collection_childs[coll_key].emplace_back(game_key);
            }
        }
    }
}

} // namespace


namespace providers {
namespace pegasus {

enum class CollAttribType : unsigned char {
    SHORT_NAME,
    LAUNCH_CMD,
    EXTENSIONS,
    FILES,
    REGEX,
};

HashMap<QString, GameFilter> read_collections_file(const HashMap<QString, CollAttribType>& key_types,
                                                   const QString& dir_path,
                                                   HashMap<QString, modeldata::Collection>& collections)
{
    // reminder: sections are collection names
    // including keys: extensions, files, regex
    // excluding keys: ignore-extensions, ignore-files, ignore-regex
    // optional: name, launch

    QString curr_config_path;
    HashMap<QString, GameFilter> config;
    modeldata::Collection* curr_coll = nullptr;

    const auto on_error = [&](const int lineno, const QString msg){
        qWarning().noquote()
            << tr_log("`%1`, line %2: %3")
                      .arg(curr_config_path, QString::number(lineno), msg);
    };
    const auto on_attribute = [&](const int lineno, const QString key, const QString val){
        if (key == QLatin1String("collection")) {
            curr_coll = nullptr;
            if (!collections.count(val))
                collections.emplace(val, modeldata::Collection(val));

            curr_coll = &collections.at(val);
            curr_coll->source_dirs.append(dir_path);
            return;
        }

        if (!curr_coll) {
            on_error(lineno, tr_log("no collection defined yet, entry ignored"));
            return;
        }

        if (key.startsWith(QLatin1String("x-"))) {
            // TODO: unimplemented
            return;
        }
        if (!key_types.count(key)) {
            on_error(lineno, tr_log("unrecognized attribute name `%3`, ignored").arg(key));
            return;
        }

        GameFilter& filter = config[curr_coll->name()];
        GameFilterGroup& filter_group = key.startsWith(QLatin1String("ignore-"))
            ? filter.exclude
            : filter.include;
        switch (key_types.at(key)) {
            case CollAttribType::SHORT_NAME:
                curr_coll->setShortName(val);
                break;
            case CollAttribType::LAUNCH_CMD:
                curr_coll->setLaunchCmd(val);
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
    if (::validFile(curr_config_path)) {
        qInfo().noquote() << tr_log("Found `%1`").arg(curr_config_path);
        config::readFile(curr_config_path,  on_attribute, on_error);
    }
    else {
        curr_config_path = dir_path + QStringLiteral("/collections.txt");
        curr_coll = nullptr;
        // FIXME: duplicate
        if (::validFile(curr_config_path)) {
            qInfo().noquote() << tr_log("Found `%1`").arg(curr_config_path);
            config::readFile(curr_config_path,  on_attribute, on_error);
        }
    }

    // cleanup and return

    auto config_it = config.begin();
    for (; config_it != config.end(); ++config_it) {
        GameFilter& filter = config_it->second;
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
        { QStringLiteral("shortname"), CollAttribType::SHORT_NAME },
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
                                      HashMap<QString, modeldata::Game>& games,
                                      HashMap<QString, modeldata::Collection>& collections,
                                      HashMap<QString, std::vector<QString>>& collection_childs,
                                      const std::function<void(int)>& update_gamecount_maybe) const
{
    for (const QString& dir_path : dir_list) {
        const auto filter_config = read_collections_file(m_key_types, dir_path, collections);
        traverse_dir(dir_path, filter_config, games, collections, collection_childs);
        update_gamecount_maybe(static_cast<int>(games.size()));
    }
}

} // namespace pegasus
} // namespace providers
