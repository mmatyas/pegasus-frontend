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
#include "model/gaming/Collection.h"
#include "model/gaming/Game.h"
#include "utils/CommandTokenizer.h"
#include "utils/PathCheck.h"
#include "utils/StdHelpers.h"

#include <QDebug>
#include <QDirIterator>
#include <QFile>
#include <QRegularExpression>
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

QVector<QStringRef> split_list(const QString& str)
{
    // FIXME: don't leave statics around
    static const QRegularExpression separator(QStringLiteral("[,\\s]"));
    return str.splitRef(separator, QString::SkipEmptyParts);
}

/// returns a list of unique, '*.'-prefixed lowercase file extensions
QStringList parseFilters(const QString& filters_raw) {
    const QString filters_lowercase = filters_raw.toLower();
    const QVector<QStringRef> filter_refs = split_list(filters_lowercase);

    QStringList filter_list;
    for (const QStringRef& filter_ref : filter_refs)
        filter_list.append(QChar('*') + filter_ref.trimmed());

    filter_list.removeDuplicates();
    return filter_list;
}


struct SystemEntry {
    const QString name;
    const QString shortname;
    const QString path;
    const QString extensions;
    const QString platforms;
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
        { QLatin1String("platform"), QString() },
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

    QString fullname = std::move(xml_props[QLatin1String("fullname")]);
    QString shortname = std::move(xml_props[QLatin1String("name")]);

    QString launch_cmd = xml_props[QLatin1String("command")]
        .replace(QLatin1String("%ROM%"), QLatin1String("{file.path}"))
        .replace(QLatin1String("%ROM_RAW%"), QLatin1String("{file.path}"))
        .replace(QLatin1String("%BASENAME%"), QLatin1String("{file.basename}"));

    return {
        fullname.isEmpty() ? shortname : fullname,
        std::move(shortname),
        std::move(xml_props[QLatin1String("path")]),
        std::move(xml_props[QLatin1String("extension")]),
        std::move(xml_props[QLatin1String("platform")]),
        std::move(launch_cmd), // assumed to be absolute
    };
}

std::vector<QString> read_mame_blacklists()
{
    using L1Str = QLatin1String;

    const QString resources_path = paths::homePath() % L1Str("/.emulationstation/resources/");
    const std::vector<std::pair<L1Str, L1Str>> blacklists {
        { L1Str("mamebioses.xml"), L1Str("bios") },
        { L1Str("mamedevices.xml"), L1Str("device") },
    };

    std::vector<QString> out;

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

        qInfo().noquote() << MSG_PREFIX
            << tr_log("found `%1`, %2 entries loaded").arg(file_path, QString::number(hit_count));
    }

    return out;
}

void find_games(const SystemEntry& sysentry, const std::vector<QString>& blacklist, providers::SearchContext& sctx)
{
    model::Collection& collection = *sctx.get_or_create_collection(sysentry.name);
    collection.setShortName(sysentry.shortname);
    collection.setCommonLaunchCmd(sysentry.launch_cmd);


    // find all (sub-)directories, but ignore 'media'
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

    // use the blacklist maybe
    const QVector<QStringRef> platforms = split_list(sysentry.platforms);
    const bool use_blacklist = VEC_CONTAINS(platforms, QLatin1String("arcade"))
                            || VEC_CONTAINS(platforms, QLatin1String("neogeo"));


    // scan for game files

    static constexpr auto entry_filters = QDir::Files | QDir::Dirs | QDir::Readable | QDir::NoDotAndDotDot;
    static constexpr auto entry_flags = QDirIterator::FollowSymlinks;
    const QStringList name_filters = parseFilters(sysentry.extensions);

    for (const QString& dir_path : qAsConst(dirs)) {
        QDirIterator files_it(dir_path, name_filters, entry_filters, entry_flags);
        while (files_it.hasNext()) {
            files_it.next();
            QFileInfo fileinfo = files_it.fileInfo();

            const QString filename = fileinfo.completeBaseName();
            if (use_blacklist && VEC_CONTAINS(blacklist, filename))
                continue;

            sctx.add_or_create_game_for(std::move(fileinfo), collection);
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
    // load the blacklist, in case it's needed
    const std::vector<QString> mame_blacklist = read_mame_blacklists();

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
        sctx.game_root_dirs.emplace_back(sysentry.path);

        find_games(sysentry, mame_blacklist, sctx);
        if (game_count != sctx.games.size()) {
            game_count = sctx.games.size();
            emit gameCountChanged(static_cast<int>(game_count));
        }
    }
}

} // namespace es2
} // namespace providers
