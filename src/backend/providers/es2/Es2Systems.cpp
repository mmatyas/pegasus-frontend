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


#include "Es2Systems.h"

#include "LocaleUtils.h"
#include "Paths.h"
#include "modeldata/gaming/Collection.h"
#include "modeldata/gaming/Game.h"
#include "utils/PathCheck.h"

#include <QDebug>
#include <QDirIterator>
#include <QFile>
#include <QStringBuilder>


namespace {
static constexpr auto MSG_PREFIX = "ES2:"; // TODO: don't duplicate

QString findSystemsFile()
{
    // static const QString FALLBACK_MSG = "`%1` not found, trying next fallback";

    const QVector<QString> possible_paths = {
        paths::homePath() % QStringLiteral("/.emulationstation/es_systems.cfg"),
        QStringLiteral("/etc/emulationstation/es_systems.cfg"),
    };

    for (const auto& path : possible_paths) {
        if (::validFile(path)) {
            qInfo().noquote() << MSG_PREFIX << tr_log("found `%1`").arg(path);
            return path;
        }
    }

    return QString();
}

HashMap<QLatin1String, QString>::iterator
findByStrRef(HashMap<QLatin1String, QString>& map, const QStringRef& str)
{
    HashMap<QLatin1String, QString>::iterator it;
    for (it = map.begin(); it != map.end(); ++it)
        if (it->first == str)
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

} // namespace


namespace providers {
namespace es2 {

SystemsParser::SystemsParser(QObject* parent)
    : QObject(parent)
{}

void SystemsParser::find(HashMap<QString, modeldata::Game>& games,
                         HashMap<QString, modeldata::Collection>& collections,
                         HashMap<QString, std::vector<QString>>& collection_childs,
                         HashMap<QString, QString>& collection_dirs)
{
    // find the systems file
    const QString xml_path = findSystemsFile();
    if (xml_path.isEmpty()) {
        qWarning().noquote() << MSG_PREFIX << tr_log("system config file not found");
        return;
    }

    // open the systems file
    QFile xml_file(xml_path);
    if (!xml_file.open(QIODevice::ReadOnly)) {
        qWarning().noquote() << MSG_PREFIX << tr_log("could not open `%1`").arg(xml_path);
        return;
    }

    // parse the systems file
    QXmlStreamReader xml(&xml_file);
    readSystemsFile(xml, games, collections, collection_childs, collection_dirs);
    if (xml.error())
        qWarning().noquote() << MSG_PREFIX << xml.errorString();
}

void SystemsParser::readSystemsFile(QXmlStreamReader& xml,
                                    HashMap<QString, modeldata::Game>& games,
                                    HashMap<QString, modeldata::Collection>& collections,
                                    HashMap<QString, std::vector<QString>>& collection_childs,
                                    HashMap<QString, QString>& collection_dirs)
{
    // read the root <systemList> element
    if (!xml.readNextStartElement()) {
        xml.raiseError(tr_log("could not parse `%1`")
                       .arg(static_cast<QFile*>(xml.device())->fileName()));
        return;
    }
    if (xml.name() != QLatin1String("systemList")) {
        xml.raiseError(tr_log("`%1` does not have a `<systemList>` root node!")
                       .arg(static_cast<QFile*>(xml.device())->fileName()));
        return;
    }

    // read all <system> nodes
    size_t game_count = games.size();
    while (xml.readNextStartElement()) {
        if (xml.name() != QLatin1String("system")) {
            xml.skipCurrentElement();
            continue;
        }

        readSystemEntry(xml, games, collections, collection_childs, collection_dirs);
        if (game_count != games.size()) {
            game_count = games.size();
            emit gameCountChanged(static_cast<int>(game_count));
        }
    }
}

void SystemsParser::readSystemEntry(QXmlStreamReader& xml,
                                    HashMap<QString, modeldata::Game>& games,
                                    HashMap<QString, modeldata::Collection>& collections,
                                    HashMap<QString, std::vector<QString>>& collection_childs,
                                    HashMap<QString, QString>& collection_dirs)
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
    HashMap<QLatin1String, QString> xml_props {
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
            it->second = xml.readElementText();
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
                << tr_log("the `<system>` node in `%1` that ends at line %2 has no `<%3>` parameter")
                   .arg(static_cast<QFile*>(xml.device())->fileName())
                   .arg(xml.lineNumber())
                   .arg(key);
            return;
        }
    }

    // do some path formatting
    xml_props[QLatin1String("path")]
        .replace("\\", "/")
        .replace("~", paths::homePath());


    // construct the new platform
    // TODO: only create if it has games

    const QString& fullname = xml_props[QLatin1String("fullname")];
    const QString& shortname = xml_props[QLatin1String("name")];
    const QString& collection_name = fullname.isEmpty() ? shortname : fullname;

    if (!collections.count(collection_name))
        collections.emplace(collection_name, modeldata::Collection(collection_name));

    modeldata::Collection& collection = collections.at(collection_name);

    collection.setShortName(shortname);
    collection_dirs[collection_name] = xml_props[QLatin1String("path")];

    const QString launch_cmd = xml_props[QLatin1String("command")]
        .replace(QLatin1String("\"%ROM%\""), QLatin1String("\"{file.path}\"")) // make sure we don't double quote
        .replace(QLatin1String("%ROM%"), QLatin1String("\"{file.path}\""))
        .replace(QLatin1String("%ROM_RAW%"), QLatin1String("{file.path}"))
        .replace(QLatin1String("%BASENAME%"), QLatin1String("{file.basename}"));
    collection.setLaunchCmd(launch_cmd);

    // add the games

    // pass 1: find all (sub-)directories, but ignore 'media'

    QStringList dirs;
    {
        static constexpr auto subdir_filters = QDir::Dirs | QDir::Readable | QDir::NoDotAndDotDot;
        static constexpr auto subdir_flags = QDirIterator::FollowSymlinks | QDirIterator::Subdirectories;

        QDirIterator dirs_it(xml_props[QLatin1String("path")], subdir_filters, subdir_flags);
        while (dirs_it.hasNext()) {
            dirs << dirs_it.next();
        }
        dirs.removeOne(xml_props[QLatin1String("path")] + QStringLiteral("/media"));
        dirs.append(xml_props[QLatin1String("path")]);
    }

    // pass 2: scan for game files

    static constexpr auto entry_filters = QDir::Files | QDir::Dirs | QDir::Readable | QDir::NoDotAndDotDot;
    static constexpr auto entry_flags = QDirIterator::FollowSymlinks;
    const QStringList name_filters = parseFilters(xml_props[QLatin1String("extension")]);

    for (const QString& dir_path : qAsConst(dirs)) {
        QDirIterator files_it(dir_path, name_filters, entry_filters, entry_flags);
        while (files_it.hasNext()) {
            files_it.next();
            QFileInfo fileinfo = files_it.fileInfo();
            const QString game_key = fileinfo.canonicalFilePath();

            if (!games.count(game_key)) {
                modeldata::Game game(std::move(fileinfo));
                game.launch_cmd = collection.launchCmd();
                games.emplace(game_key, std::move(game));
            }
            collection_childs[collection_name].emplace_back(game_key);
        }
    }
}

} // namespace es2
} // namespace providers
