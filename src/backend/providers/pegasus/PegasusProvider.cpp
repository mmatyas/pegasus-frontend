// Pegasus Frontend
// Copyright (C) 2017  Mátyás Mustoha
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

#include "ConfigFile.h"
#include "Utils.h"
#include "types/Collection.h"

#include <QDebug>
#include <QDirIterator>
#include <QFile>
#include <QRegularExpression>
#include <QTextStream>


namespace {

QStringList load_game_dir_list()
{
    constexpr int LINE_MAX_LEN = 4096;

    QStringList rom_dirs;
    for (QString& path : ::configDirPaths()) {
        path += QStringLiteral("/game_dirs.txt");

        QFile config_file(path);
        if (!config_file.open(QFile::ReadOnly | QFile::Text))
            continue;

        qInfo() << QObject::tr("Found `%1`").arg(path);

        QTextStream stream(&config_file);
        QString line;
        while (stream.readLineInto(&line, LINE_MAX_LEN)) {
            if (!line.startsWith('#'))
                rom_dirs << line;
        }
    }

    return rom_dirs;
}

enum class AttribType : unsigned char {
    NAME,
    LAUNCH_CMD,
    EXTENSIONS,
    FILES,
    REGEX,
};
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

QHash<QString, GameFilter> read_collections_file(const QString& dir_path,
                                                 QHash<QString, Types::Collection*>& collections)
{
    // reminder: sections are collection tags
    // including keys: extensions, files, regex
    // excluding keys: ignore-extensions, ignore-files, ignore-regex
    // optional: name, launch

    static const QHash<QString, AttribType> key_types {
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
    };

    QString curr_file_path;
    QString curr_coll_name;
    QHash<QString, GameFilter> config;

    static const auto on_section = [&](const int, const QString name){
        curr_coll_name = name;

        if (!collections.contains(name))
            collections.insert(name, new Types::Collection(name));

        collections[name]->sourceDirsMut().append(dir_path);
    };
    static const auto on_attribute = [&](const int lineno, const QString key, const QString val){
        if (curr_coll_name.isEmpty()) {
            qWarning().noquote()
                << QObject::tr("`%1`, line %2: no sections defined yet, values ignored")
                               .arg(curr_file_path, QString::number(lineno));
            return;
        }

        GameFilter& filter = config[curr_coll_name];
        if (key.startsWith(QLatin1String("x-"))) {
            // TODO: unimplemented
            return;
        }
        if (!key_types.contains(key)) {
            qWarning().noquote()
                << QObject::tr("`%1`, line %2: unrecognized attribute name, ignored")
                               .arg(curr_file_path, QString::number(lineno));
            return;
        }

        GameFilterGroup& filter_group = key.startsWith(QLatin1String("ignore-"))
            ? filter.exclude
            : filter.include;
        switch (key_types[key]) {
            case AttribType::NAME:
                collections[curr_coll_name]->setName(val);
                break;
            case AttribType::LAUNCH_CMD:
                collections[curr_coll_name]->setCommonLaunchCmd(val);
                break;
            case AttribType::EXTENSIONS:
                filter_group.extensions.append(::tokenize(val.toLower()));
                break;
            case AttribType::FILES:
                filter_group.files.append(::tokenize(val));
                break;
            case AttribType::REGEX:
                if (!filter_group.regex.isEmpty()) {
                    qWarning().noquote()
                        << QObject::tr("`%1`, line %2: `%3` was already defined for this collection, replaced")
                                       .arg(curr_file_path, QString::number(lineno), key);
                }
                filter_group.regex = val;
                break;
        }

    };
    static const auto on_error = [&](const int lineno, const QString msg){
        qWarning().noquote()
            << QObject::tr("`%1`, line %2: %3")
                           .arg(curr_file_path, QString::number(lineno), msg);
    };


    // the actual reading

    curr_file_path = dir_path + QStringLiteral("/collections.pegasus.txt");
    config::readFile(curr_file_path, on_section, on_attribute, on_error);

    curr_file_path = dir_path + QStringLiteral("/collections.txt");
    config::readFile(curr_file_path, on_section, on_attribute, on_error);

    return config;
}

void traverse_dir(const QString& dir_path,
                  const QHash<QString, GameFilter>& filter_config,
                  QHash<QString, Types::Game*>& games,
                  QHash<QString, Types::Collection*>& collections)
{
    auto config_it = filter_config.constBegin();
    while (config_it != filter_config.constEnd()) {
        const GameFilter& filter = config_it.value();
        const QRegularExpression include_regex(filter.include.regex);
        const QRegularExpression exclude_regex(filter.exclude.regex);

        static constexpr auto entry_filters = QDir::Files | QDir::Dirs | QDir::Readable | QDir::NoDotAndDotDot;
        static constexpr auto entry_flags = QDirIterator::FollowSymlinks;

        QDirIterator dir_it(dir_path, entry_filters, entry_flags);
        while (dir_it.hasNext()) {
            dir_it.next();
            const QFileInfo fileinfo = dir_it.fileInfo();

            const bool exclude = filter.exclude.extensions.contains(fileinfo.suffix())
                || filter.exclude.files.contains(fileinfo.filePath())
                || exclude_regex.match(fileinfo.filePath()).hasMatch();
            if (exclude)
                continue;

            const bool include = filter.include.extensions.contains(fileinfo.suffix())
                || filter.include.files.contains(fileinfo.filePath())
                || include_regex.match(fileinfo.filePath()).hasMatch();
            if (!include)
                continue;

            Types::Collection*& collection_ptr = collections[config_it.key()];
            Types::Game*& game_ptr = games[fileinfo.canonicalFilePath()];
            if (!game_ptr)
                game_ptr = new Types::Game(fileinfo, collection_ptr);

            collection_ptr->gameListMut().addGame(game_ptr);
        }
    }
}

} // namespace


namespace providers {
namespace pegasus {

PegasusProvider::PegasusProvider(QObject* parent)
    : Provider(parent)
{}

void PegasusProvider::find(QHash<QString, Types::Game*>& games,
                           QHash<QString, Types::Collection*>& collections)
{
    const QStringList game_dirs = load_game_dir_list();
    for (const QString& dir_path : game_dirs) {
        const auto filter_config = read_collections_file(dir_path, collections);
        traverse_dir(dir_path, filter_config, games, collections);
    }
}

void PegasusProvider::enhance(const QHash<QString, Types::Game*>&,
             const QHash<QString, Types::Collection*>&)
{}

void PegasusProvider::find_in_thirdparty_dirs(const QStringList&,
                             QHash<QString, Types::Game*>&,
                             QHash<QString, Types::Collection*>&)
{}

} // namespace pegasus
} // namespace providers
