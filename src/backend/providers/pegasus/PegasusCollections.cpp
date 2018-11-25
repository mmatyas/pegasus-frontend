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
#include "Paths.h"
#include "PegasusAssets.h"
#include "PegasusCommon.h"
#include "modeldata/gaming/Collection.h"
#include "modeldata/gaming/Game.h"
#include "utils/PathCheck.h"

#include <QDebug>
#include <QDirIterator>
#include <QRegularExpression>
#include <QStringBuilder>


namespace providers {
namespace pegasus {

enum class CollAttribType : unsigned char {
    SHORT_NAME,
    DIRECTORIES,
    EXTENSIONS,
    FILES,
    REGEX,
    SHORT_DESC,
    LONG_DESC,
    LAUNCH_CMD,
    LAUNCH_WORKDIR,
};

} // namespace providers
} //namespace pegasus


namespace {
static constexpr auto MSG_PREFIX = "Collections:";

using AttribType = providers::pegasus::CollAttribType;

struct GameFilterGroup {
    QStringList extensions;
    QStringList files;
    QString regex;
};
struct GameFilter {
    QString parent_collection;
    QStringList directories;
    GameFilterGroup include;
    GameFilterGroup exclude;
    QStringList extra;

    GameFilter(QString parent, QString base_dir)
        : parent_collection(parent)
        , directories(base_dir)
    {}
};

std::vector<GameFilter> read_collections_file(const HashMap<QString, AttribType>& key_types,
                                              const QString& dir_path,
                                              HashMap<QString, modeldata::Collection>& collections)
{
    // reminder: sections are collection names
    // including keys: extensions, files, regex
    // excluding keys: ignore-extensions, ignore-files, ignore-regex
    // optional: name, launch, directories

    const QRegularExpression rx_asset_key(QStringLiteral(R"(^assets?\.default-?(.+)$)"));

    QString curr_config_path;
    std::vector<GameFilter> filters;
    modeldata::Collection* curr_coll = nullptr;

    const auto on_error = [&](const int lineno, const QString msg){
        qWarning().noquote() << MSG_PREFIX
            << tr_log("`%1`, line %2: %3")
                      .arg(curr_config_path, QString::number(lineno), msg);
    };
    const auto on_attribute = [&](const int lineno, const QString key, const QString val){
        if (key == QLatin1String("collection")) {
            curr_coll = nullptr;
            if (!collections.count(val))
                collections.emplace(val, modeldata::Collection(val));

            curr_coll = &collections.at(val);
            filters.emplace_back(val, dir_path);
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

        const auto rx_asset = rx_asset_key.match(key);
        if (rx_asset.hasMatch()) {
            const QString asset_key = rx_asset.captured(1);
            const AssetType asset_type = pegasus_assets::str_to_type(asset_key);
            if (asset_type == AssetType::UNKNOWN) {
                on_error(lineno, tr_log("unknown asset type '%1', entry ignored").arg(asset_key));
                return;
            }

            providers::pegasus::add_asset(curr_coll->default_assets, asset_type, val, dir_path);
            return;
        }
        if (!key_types.count(key)) {
            on_error(lineno, tr_log("unrecognized attribute name `%3`, ignored").arg(key));
            return;
        }

        GameFilter& filter = filters.back();
        GameFilterGroup& filter_group = key.startsWith(QLatin1String("ignore-"))
            ? filter.exclude
            : filter.include;

        switch (key_types.at(key)) {
            case AttribType::SHORT_NAME:
                curr_coll->setShortName(val);
                break;
            case AttribType::LAUNCH_CMD:
                curr_coll->setLaunchCmd(val);
                break;
            case AttribType::DIRECTORIES:
                {
                    QFileInfo finfo(val);
                    if (finfo.isRelative())
                        finfo.setFile(dir_path % '/' % val);

                    filter.directories.append(finfo.canonicalFilePath());
                }
                break;
            case AttribType::EXTENSIONS:
                filter_group.extensions.append(providers::pegasus::tokenize(val.toLower()));
                break;
            case AttribType::FILES:
                filter_group.files.append(val);
                break;
            case AttribType::REGEX:
                filter_group.regex = val;
                break;
            case AttribType::SHORT_DESC:
                curr_coll->summary = val;
                break;
            case AttribType::LONG_DESC:
                curr_coll->description = val;
                break;
            case AttribType::LAUNCH_WORKDIR:
                curr_coll->launch_workdir = val;
                break;
        }
    };


    // the actual reading

    const QStringList possible_paths {
        dir_path + QStringLiteral("/collections.pegasus.txt"),
        dir_path + QStringLiteral("/collections.txt"),
    };
    for (const QString& path : possible_paths) {
        if (!::validFile(path))
            continue;

        qInfo().noquote() << MSG_PREFIX << tr_log("found `%1`").arg(path);

        curr_coll = nullptr;
        curr_config_path = path;
        config::readFile(path,  on_attribute, on_error);
        break; // if the first file exists, don't check the other
    }

    // cleanup and return

    for (GameFilter& filter : filters) {
        filter.directories.removeDuplicates();
        filter.include.extensions.removeDuplicates();
        filter.include.files.removeDuplicates();
        filter.exclude.extensions.removeDuplicates();
        filter.exclude.files.removeDuplicates();
        filter.extra.removeDuplicates();
    }
    return filters;
}

void process_filter(const GameFilter& filter,
                    HashMap<QString, modeldata::Game>& games,
                    HashMap<QString, modeldata::Collection>& collections,
                    HashMap<QString, std::vector<QString>>& collection_childs)
{
    constexpr auto entry_filters = QDir::Files | QDir::Dirs | QDir::Readable | QDir::NoDotAndDotDot;
    constexpr auto subdir_filters = QDir::Dirs | QDir::Readable | QDir::NoDotAndDotDot;
    constexpr auto entry_flags = QDirIterator::FollowSymlinks;
    constexpr auto subdir_flags = QDirIterator::FollowSymlinks | QDirIterator::Subdirectories;

    const QRegularExpression include_regex(filter.include.regex);
    const QRegularExpression exclude_regex(filter.exclude.regex);


    for (const QString& filter_dir : filter.directories)
    {
        // find all dirs and subdirectories, but ignore 'media'
        QStringList dirs_to_check;
        {
            QDirIterator dirs_it(filter_dir, subdir_filters, subdir_flags);
            while (dirs_it.hasNext())
                dirs_to_check << dirs_it.next();

            dirs_to_check.removeOne(filter_dir + QStringLiteral("/media"));
            dirs_to_check.append(filter_dir + QStringLiteral("/")); // added "/" so all entries have base + 1 length
        }

        // run through the directories
        for (const QString& subdir : qAsConst(dirs_to_check)) {
            QDirIterator subdir_it(subdir, entry_filters, entry_flags);
            while (subdir_it.hasNext()) {
                subdir_it.next();
                QFileInfo fileinfo = subdir_it.fileInfo();
                const QString relative_path = fileinfo.filePath().mid(filter_dir.length() + 1);

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

                const QString game_key = fileinfo.canonicalFilePath();
                if (!games.count(game_key)) {
                    modeldata::Game game(std::move(fileinfo));
                    game.launch_cmd = collections.at(filter.parent_collection).launchCmd();
                    games.emplace(game_key, std::move(game));
                }
                collection_childs[filter.parent_collection].emplace_back(game_key);
            }
        }
    }
}

} // namespace


namespace providers {
namespace pegasus {

PegasusCollections::PegasusCollections()
    : m_key_types {
        { QStringLiteral("shortname"), AttribType::SHORT_NAME },
        { QStringLiteral("launch"), AttribType::LAUNCH_CMD },
        { QStringLiteral("command"), AttribType::LAUNCH_CMD },
        { QStringLiteral("directory"), AttribType::DIRECTORIES },
        { QStringLiteral("directories"), AttribType::DIRECTORIES },
        { QStringLiteral("extension"), AttribType::EXTENSIONS },
        { QStringLiteral("extensions"), AttribType::EXTENSIONS },
        { QStringLiteral("file"), AttribType::FILES },
        { QStringLiteral("files"), AttribType::FILES },
        { QStringLiteral("regex"), AttribType::REGEX },
        { QStringLiteral("ignore-extension"), AttribType::EXTENSIONS },
        { QStringLiteral("ignore-extensions"), AttribType::EXTENSIONS },
        { QStringLiteral("ignore-file"), AttribType::FILES },
        { QStringLiteral("ignore-files"), AttribType::FILES },
        { QStringLiteral("ignore-regex"), AttribType::REGEX },
        { QStringLiteral("summary"), AttribType::SHORT_DESC },
        { QStringLiteral("description"), AttribType::LONG_DESC },
        { QStringLiteral("workdir"), AttribType::LAUNCH_WORKDIR },
        { QStringLiteral("working-directory"), AttribType::LAUNCH_WORKDIR },
        { QStringLiteral("cwd"), AttribType::LAUNCH_WORKDIR },
    }
{
}

void PegasusCollections::find_in_dirs(const std::vector<QString>& dir_list,
                                      HashMap<QString, modeldata::Game>& games,
                                      HashMap<QString, modeldata::Collection>& collections,
                                      HashMap<QString, std::vector<QString>>& collection_childs,
                                      const std::function<void(int)>& update_gamecount_maybe) const
{
    std::vector<GameFilter> all_filters;

    for (const QString& dir_path : dir_list) {
        auto filters = read_collections_file(m_key_types, dir_path, collections);
        all_filters.reserve(all_filters.size() + filters.size());
        all_filters.insert(all_filters.end(),
                           std::make_move_iterator(filters.begin()),
                           std::make_move_iterator(filters.end()));
    }
    for (const GameFilter& filter : all_filters)
        process_filter(filter, games, collections, collection_childs);

    update_gamecount_maybe(static_cast<int>(games.size()));
}

} // namespace pegasus
} // namespace providers
