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


#include "Es2GamelistParser.h"

#include "Utils.h"
#include "types/Collection.h"
#include "types/Game.h"

#include <QDebug>
#include <QDirIterator>
#include <QFile>


namespace {
static constexpr auto MSG_PREFIX = "ES2:"; // TODO: don't duplicate
} // namespace

namespace providers {
namespace es2_utils {

void readSystemsFile(QXmlStreamReader& xml,
                     QHash<QString, Types::Game*>& games,
                     QHash<QString, Types::Collection*>& collections,
                     QVector<QString>& metadata_dirs,
                     const std::function<void(int)>& onGamesChangedCB)
{
    // read the root <systemList> element
    if (!xml.readNextStartElement()) {
        xml.raiseError(QObject::tr("could not parse `%1`")
                       .arg(static_cast<QFile*>(xml.device())->fileName()));
        return;
    }
    if (xml.name() != QLatin1String("systemList")) {
        xml.raiseError(QObject::tr("`%1` does not have a `<systemList>` root node!")
                       .arg(static_cast<QFile*>(xml.device())->fileName()));
        return;
    }

    // read all <system> nodes
    int game_count = games.count();
    while (xml.readNextStartElement()) {
        if (xml.name() != QLatin1String("system")) {
            xml.skipCurrentElement();
            continue;
        }

        readSystemEntry(xml, games, collections, metadata_dirs);
        if (game_count != games.count()) {
            game_count = games.count();
            onGamesChangedCB(game_count);
        }
    }
}


void readSystemEntry(QXmlStreamReader& xml,
                     QHash<QString, Types::Game*>& games,
                     QHash<QString, Types::Collection*>& collections,
                     QVector<QString>& metadata_dirs)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == "system");

    // read all XML fields into a key-value map

    // non-optional properties
    static const QVector<QLatin1String> required_keys {{
        QLatin1String("name"),
        QLatin1String("path"),
        QLatin1String("extension"),
        QLatin1String("command"),
    }};
    // all supported properties
    QHash<QLatin1String, QString> xml_props {
        { QLatin1String("name"), QString() },
        { QLatin1String("fullname"), QString() },
        { QLatin1String("path"), QString() },
        { QLatin1String("extension"), QString() },
        { QLatin1String("command"), QString() },
    };
    // read
    while (xml.readNextStartElement()) {
        auto it = findByStrRef(xml_props, xml.name());
        if (it != xml_props.end())
            it.value() = xml.readElementText();
        else
            xml.skipCurrentElement();
    }
    if (xml.error()) {
        qWarning().noquote() << MSG_PREFIX << xml.errorString();
        return;
    }

    // check if all required params are present
    for (const auto& key : required_keys) {
        if (xml_props[key].isEmpty()) {
            qWarning().noquote()
                << MSG_PREFIX
                << QObject::tr("the `<system>` node in `%1` that ends at line %2 has no `<%3>` parameter")
                   .arg(static_cast<QFile*>(xml.device())->fileName())
                   .arg(xml.lineNumber())
                   .arg(key);
            return;
        }
    }

    // do some path formatting
    xml_props[QLatin1String("path")]
        .replace("\\", "/")
        .replace("~", homePath());


    // construct the new platform
    // TODO: only create if it has games

    const QString& tag = xml_props[QLatin1String("name")];
    Types::Collection*& collection = collections[tag];
    if (!collection)
        collection = new Types::Collection(tag); // TODO: check for fail

    collection->setName(xml_props[QLatin1String("fullname")]);
    collection->setCommonLaunchCmd(xml_props[QLatin1String("command")]);
    collection->sourceDirsMut().append(xml_props[QLatin1String("path")]);

    // add the games

    static const auto dir_filters = QDir::Dirs | QDir::Readable | QDir::NoDotAndDotDot;
    static const auto file_filters = QDir::Files | QDir::Readable | QDir::NoDotAndDotDot;
    static const auto dir_flags = QDirIterator::FollowSymlinks | QDirIterator::Subdirectories;
    static const auto file_flags = QDirIterator::FollowSymlinks;
    const QStringList name_filters = parseFilters(xml_props[QLatin1String("extension")]);

    // pass 1: find all (sub-)directories, but ignore 'media'
    QStringList dirs;
    QDirIterator dirs_it(xml_props[QLatin1String("path")], dir_filters, dir_flags);
    while (dirs_it.hasNext()) {
        dirs << dirs_it.next();
    }
    dirs.removeOne(xml_props[QLatin1String("path")] + QStringLiteral("/media"));
    dirs.append(xml_props[QLatin1String("path")]);

    // pass 2: scan for game files
    for (const QString& dir_path : qAsConst(dirs)) {
        QDirIterator files_it(dir_path, name_filters, file_filters, file_flags);
        while (files_it.hasNext()) {
            files_it.next();
            QFileInfo fileinfo = files_it.fileInfo();

            Types::Game*& game_ptr = games[fileinfo.canonicalFilePath()];
            if (!game_ptr)
                game_ptr = new Types::Game(fileinfo, collection);

            collection->gameListMut().addGame(game_ptr);
        }
    }

    // search for metadata here in the next phase
    metadata_dirs << xml_props[QLatin1String("path")];
}

QHash<QLatin1String, QString>::iterator findByStrRef(QHash<QLatin1String, QString>& map, const QStringRef& str)
{
    QHash<QLatin1String, QString>::iterator it;
    for (it = map.begin(); it != map.end(); ++it)
        if (it.key() == str)
            break;

    return it;
}

/// returns a list of unique, '*.'-prefixed lowercase file extensions
QStringList parseFilters(const QString& filters_raw) {
    QStringList filter_list = filters_raw.split(" ", QString::SkipEmptyParts);
    for (auto& filter : filter_list)
        filter = filter.prepend("*").toLower();

    filter_list.removeDuplicates();
    return filter_list;
}

} // namespace es2_utils
} // namespace providers
