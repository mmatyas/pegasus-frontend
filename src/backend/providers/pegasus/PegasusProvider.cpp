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
#include "types/Game.h"
#include "types/Collection.h"

#include <QDebug>
#include <QDirIterator>
#include <QFile>
#include <QRegularExpression>
#include <QStringBuilder>
#include <QTextStream>


namespace {

QStringList tokenize(const QString& str)
{
    QStringList list = str.split(',', QString::SkipEmptyParts);
    for (QString& item : list)
        item = item.trimmed();

    return list;
}

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

enum class CollAttribType : unsigned char {
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

    static const QHash<QString, CollAttribType> key_types {
        { QStringLiteral("name"), CollAttribType::NAME },
        { QStringLiteral("launch"), CollAttribType::LAUNCH_CMD },
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
    };

    QString curr_file_path;
    QString curr_coll_name;
    QHash<QString, GameFilter> config;

    const auto on_section = [&](const int, const QString name){
        curr_coll_name = name;

        if (!collections.contains(name))
            collections.insert(name, new Types::Collection(name));

        collections[name]->sourceDirsMut().append(dir_path);
    };
    const auto on_attribute = [&](const int lineno, const QString key, const QString val){
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
                << QObject::tr("`%1`, line %2: unrecognized attribute name `%3`, ignored")
                               .arg(curr_file_path, QString::number(lineno), key);
            return;
        }

        GameFilterGroup& filter_group = key.startsWith(QLatin1String("ignore-"))
            ? filter.exclude
            : filter.include;
        switch (key_types[key]) {
            case CollAttribType::NAME:
                collections[curr_coll_name]->setName(val);
                break;
            case CollAttribType::LAUNCH_CMD:
                collections[curr_coll_name]->setCommonLaunchCmd(val);
                break;
            case CollAttribType::EXTENSIONS:
                filter_group.extensions.append(tokenize(val.toLower()));
                break;
            case CollAttribType::FILES:
                filter_group.files.append(tokenize(val));
                break;
            case CollAttribType::REGEX:
                if (!filter_group.regex.isEmpty()) {
                    qWarning().noquote()
                        << QObject::tr("`%1`, line %2: `%3` was already defined for this collection, replaced")
                                       .arg(curr_file_path, QString::number(lineno), key);
                }
                filter_group.regex = val;
                break;
        }

    };
    const auto on_error = [&](const int lineno, const QString msg){
        qWarning().noquote()
            << QObject::tr("`%1`, line %2: %3")
                           .arg(curr_file_path, QString::number(lineno), msg);
    };


    // the actual reading

    curr_file_path = dir_path + QStringLiteral("/collections.pegasus.txt");
    config::readFile(curr_file_path, on_section, on_attribute, on_error);

    curr_file_path = dir_path + QStringLiteral("/collections.txt");
    curr_coll_name.clear();
    config::readFile(curr_file_path, on_section, on_attribute, on_error);

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

enum class MetaAttribType {
    TITLE,
    DEVELOPER,
    PUBLISHER,
    GENRE,
    PLAYER_COUNT,
    SHORT_DESC,
    LONG_DESC,
    RELEASE,
    RATING,
};

void read_metadata_file(const QString& dir_path,
                        const QHash<QString, Types::Game*>& games)
{
    // reminder: sections are relative file paths

    static const QHash<QString, MetaAttribType> key_types {
        { QStringLiteral("title"), MetaAttribType::TITLE },
        { QStringLiteral("name"), MetaAttribType::TITLE },
        { QStringLiteral("developer"), MetaAttribType::DEVELOPER },
        { QStringLiteral("developers"), MetaAttribType::DEVELOPER },
        { QStringLiteral("publisher"), MetaAttribType::PUBLISHER },
        { QStringLiteral("publishers"), MetaAttribType::PUBLISHER },
        { QStringLiteral("genre"), MetaAttribType::GENRE },
        { QStringLiteral("genres"), MetaAttribType::GENRE },
        { QStringLiteral("players"), MetaAttribType::PLAYER_COUNT },
        { QStringLiteral("summary"), MetaAttribType::SHORT_DESC },
        { QStringLiteral("description"), MetaAttribType::LONG_DESC },
        { QStringLiteral("release"), MetaAttribType::RELEASE },
        { QStringLiteral("rating"), MetaAttribType::RATING },
    };
    static const QRegularExpression PLAYERS_REGEX(QStringLiteral("(\\d+)(-(\\d+))?"));
    static const QRegularExpression RATING_REGEX(QStringLiteral("(\\d+(\\.\\d+)?)%"));
    static const QRegularExpression RELEASE_REGEX(QStringLiteral("(\\d{4})(-(\\d{1,2}))?(-(\\d{1,2}))?"));

    QString curr_file_path;
    QString curr_game_path;
    Types::Game* curr_game = nullptr;

    const auto on_error = [&](const int lineno, const QString msg){
        qWarning().noquote()
            << QObject::tr("`%1`, line %2: %3")
                           .arg(curr_file_path, QString::number(lineno), msg);
    };
    const auto on_section = [&](const int lineno, const QString name){
        curr_game_path = dir_path % '/' % name;
        const QFileInfo fileinfo(curr_game_path);
        curr_game = nullptr;

        if (!games.contains(fileinfo.canonicalFilePath())) {
            on_error(lineno,
                QObject::tr("the game `%3` is either missing or excluded, values for it will be ignored").arg(name));
            return;
        }

        curr_game = games[fileinfo.canonicalFilePath()];
        Q_ASSERT(curr_game);
    };
    const auto on_attribute = [&](const int lineno, const QString key, const QString val){
        if (curr_game_path.isEmpty()) {
            on_error(lineno, QObject::tr("no sections defined yet, values ignored"));
            return;
        }
        if (!curr_game)
            return;

        if (key.startsWith(QLatin1String("x-"))) {
            // TODO: unimplemented
            return;
        }
        if (!key_types.contains(key)) {
            on_error(lineno, QObject::tr("unrecognized attribute name `%3`, ignored").arg(key));
            return;
        }

        switch (key_types[key]) {
            case MetaAttribType::TITLE:
                curr_game->m_title = val;
                break;
            case MetaAttribType::DEVELOPER:
                curr_game->addDevelopers(tokenize(val));
                break;
            case MetaAttribType::PUBLISHER:
                curr_game->addPublishers(tokenize(val));
                break;
            case MetaAttribType::GENRE:
                curr_game->addGenres(tokenize(val));
                break;
            case MetaAttribType::PLAYER_COUNT:
                {
                    const auto rx_match = PLAYERS_REGEX.match(val);
                    if (rx_match.hasMatch()) {
                        const int a = rx_match.capturedRef(1).toInt();
                        const int b = rx_match.capturedRef(3).toInt();
                        curr_game->m_players = qMax(1, qMax(a, b));
                    }
                }
                break;
            case MetaAttribType::SHORT_DESC:
                curr_game->m_summary = val;
                break;
            case MetaAttribType::LONG_DESC:
                curr_game->m_description = val;
                break;
            case MetaAttribType::RELEASE:
                {
                    const auto rx_match = RELEASE_REGEX.match(val);
                    if (!rx_match.hasMatch()) {
                        on_error(lineno, QObject::tr("incorrect date format, should be YYYY(-MM(-DD))"));
                        return;
                    }

                    const int y = qMax(1, rx_match.captured(1).toInt());
                    const int m = qBound(1, rx_match.captured(3).toInt(), 12);
                    const int d = qBound(1, rx_match.captured(5).toInt(), 31);
                    curr_game->setRelease(QDate(y, m, d));
                }
                break;
            case MetaAttribType::RATING:
                {
                    const auto rx_match_a = RATING_REGEX.match(val);
                    if (!rx_match_a.hasMatch()) {
                        on_error(lineno, QObject::tr("failed to parse rating value"));
                        return;
                    }

                    curr_game->m_rating = qBound(0.f, rx_match_a.capturedRef(1).toFloat() / 100.f, 1.f);
                }
                break;
        }
    };


    // the actual reading

    curr_file_path = dir_path + QStringLiteral("/metadata.pegasus.txt");
    config::readFile(curr_file_path, on_section, on_attribute, on_error);

    curr_file_path = dir_path + QStringLiteral("/metadata.txt");
    curr_game = nullptr;
    config::readFile(curr_file_path, on_section, on_attribute, on_error);
}

} // namespace


namespace providers {
namespace pegasus {

PegasusProvider::PegasusProvider(QObject* parent)
    : Provider(parent)
    , m_game_dirs(load_game_dir_list())
{}

void PegasusProvider::find(QHash<QString, Types::Game*>& games,
                           QHash<QString, Types::Collection*>& collections)
{
    find_in_dirs(m_game_dirs, games, collections);
}

void PegasusProvider::enhance(const QHash<QString, Types::Game*>& games,
                              const QHash<QString, Types::Collection*>& collections)
{
    enhance_in_dirs(m_game_dirs, games, collections);
}

void PegasusProvider::find_in_dirs(const QStringList& dir_list,
                                   QHash<QString, Types::Game*>& games,
                                   QHash<QString, Types::Collection*>& collections)
{
    for (const QString& dir_path : dir_list) {
        const auto filter_config = read_collections_file(dir_path, collections);
        traverse_dir(dir_path, filter_config, collections, games);
    }
}

void PegasusProvider::enhance_in_dirs(const QStringList& dir_list,
                                      const QHash<QString, Types::Game*>& games,
                                      const QHash<QString, Types::Collection*>&)
{
    for (const QString& dir_path : dir_list) {
        read_metadata_file(dir_path, games);
    }
}

} // namespace pegasus
} // namespace providers
