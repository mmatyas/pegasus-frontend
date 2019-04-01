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


#include "Es2Systems.h"

#include "LocaleUtils.h"
#include "Paths.h"
#include "modeldata/CollectionData.h"
#include "modeldata/GameData.h"
#include "utils/CommandTokenizer.h"
#include "utils/PathCheck.h"

#include <QDebug>
#include <QDirIterator>
#include <QFile>
#include <QStringBuilder>


namespace {
static constexpr auto MSG_PREFIX = "EmulationStation:"; // TODO: don't duplicate

QString findSystemsFile()
{
    // static const QString FALLBACK_MSG = "`%1` not found, trying next fallback";

    const QVector<QString> possible_paths {
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


struct SystemEntry {
    const QString name;
    const QString shortname;
    const QString path;
    const QString extensions;
    const QString launch_cmd;
};

SystemEntry read_system_entry(QXmlStreamReader& xml)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == "system");

    // read all XML fields into a key-value map

    // non-optional properties
    const std::vector<QLatin1String> required_keys {{
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
        return {};
    }

    // check if all required params are present
    for (const QLatin1String& key : required_keys) {
        if (xml_props[key].isEmpty()) {
            qWarning().noquote()
                << MSG_PREFIX
                << tr_log("the `<system>` node in `%1` that ends at line %2 has no `<%3>` parameter")
                   .arg(static_cast<QFile*>(xml.device())->fileName())
                   .arg(xml.lineNumber())
                   .arg(key);
            return {};
        }
    }

    // do some path formatting
    xml_props[QLatin1String("path")]
        .replace("\\", "/")
        .replace("~", paths::homePath());


    // construct the new platform

    const QString& fullname = xml_props[QLatin1String("fullname")];
    const QString& shortname = xml_props[QLatin1String("name")];

    const QString launch_cmd = xml_props[QLatin1String("command")]
        .replace(QLatin1String("%ROM%"), QLatin1String("{file.path}"))
        .replace(QLatin1String("%ROM_RAW%"), QLatin1String("{file.path}"))
        .replace(QLatin1String("%BASENAME%"), QLatin1String("{file.basename}"));

    return {
        fullname.isEmpty() ? shortname : fullname,
        shortname,
        xml_props[QLatin1String("path")],
        xml_props[QLatin1String("extension")],
        launch_cmd, // assumed to be absolute
    };
}

void find_games(const SystemEntry& sysentry, providers::SearchContext& sctx)
{
    // add the collection

    auto collection_it = sctx.collections.find(sysentry.name);
    if (collection_it == sctx.collections.end())
        collection_it = sctx.collections.emplace(sysentry.name, modeldata::Collection(sysentry.name)).first;

    modeldata::Collection& collection = collection_it->second;
    std::vector<size_t>& childs = sctx.collection_childs[sysentry.name];

    collection.setShortName(sysentry.shortname);
    collection.launch_cmd = sysentry.launch_cmd;


    // add the games

    // pass 1: find all (sub-)directories, but ignore 'media'

    QStringList dirs;
    {
        static constexpr auto subdir_filters = QDir::Dirs | QDir::Readable | QDir::NoDotAndDotDot;
        static constexpr auto subdir_flags = QDirIterator::FollowSymlinks | QDirIterator::Subdirectories;

        QDirIterator dirs_it(sysentry.path, subdir_filters, subdir_flags);
        while (dirs_it.hasNext()) {
            dirs << dirs_it.next();
        }
        dirs.removeOne(sysentry.path + QStringLiteral("/media"));
        dirs.append(sysentry.path);
    }

    // pass 2: scan for game files

    static constexpr auto entry_filters = QDir::Files | QDir::Dirs | QDir::Readable | QDir::NoDotAndDotDot;
    static constexpr auto entry_flags = QDirIterator::FollowSymlinks;
    const QStringList name_filters = parseFilters(sysentry.extensions);

    for (const QString& dir_path : qAsConst(dirs)) {
        QDirIterator files_it(dir_path, name_filters, entry_filters, entry_flags);
        while (files_it.hasNext()) {
            files_it.next();
            QFileInfo fileinfo = files_it.fileInfo();
            const QString game_path = fileinfo.canonicalFilePath();

            if (!sctx.path_to_gameid.count(game_path)) {
                modeldata::Game game(fileinfo);
                game.launch_cmd = collection.launch_cmd;

                const size_t game_id = sctx.games.size();
                sctx.path_to_gameid.emplace(game_path, game_id);
                sctx.games.emplace(game_id, std::move(game));
            }

            const size_t game_id = sctx.path_to_gameid.at(game_path);
            childs.emplace_back(game_id);
        }
    }
}
} // namespace


namespace providers {
namespace es2 {

SystemsParser::SystemsParser(QObject* parent)
    : QObject(parent)
{}

void SystemsParser::find(providers::SearchContext& sctx,
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
    read_systems_file(xml, sctx, collection_dirs);
    if (xml.error())
        qWarning().noquote() << MSG_PREFIX << xml.errorString();
}

void SystemsParser::read_systems_file(QXmlStreamReader& xml,
                                      providers::SearchContext& sctx,
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
    size_t game_count = sctx.games.size();
    while (xml.readNextStartElement()) {
        if (xml.name() != QLatin1String("system")) {
            xml.skipCurrentElement();
            continue;
        }

        const SystemEntry sysentry = read_system_entry(xml);
        if (sysentry.name.isEmpty())
            continue;

        collection_dirs[sysentry.name] = sysentry.path;

        find_games(sysentry, sctx);
        if (game_count != sctx.games.size()) {
            game_count = sctx.games.size();
            emit gameCountChanged(static_cast<int>(game_count));
        }
    }
}

} // namespace es2
} // namespace providers
