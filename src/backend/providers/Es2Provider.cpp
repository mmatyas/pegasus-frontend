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


#include "Es2Provider.h"

#include "types/Collection.h"
#include "types/Game.h"
#include "Utils.h"

#include <QDebug>
#include <QDirIterator>
#include <QFile>
#include <QStringBuilder>
#include <QXmlStreamReader>


namespace {

static constexpr auto MSG_PREFIX = "ES2:";

void readSystemsFile(QXmlStreamReader& xml,
                     QHash<QString, Types::Game*>& games,
                     QHash<QString, Types::Collection*>& collections,
                     QVector<QString>& metadata_dirs);
void readSystemEntry(QXmlStreamReader& xml,
                     QHash<QString, Types::Game*>& games,
                     QHash<QString, Types::Collection*>& collections,
                     QVector<QString>& metadata_dirs);
QHash<QLatin1String, QString>::iterator findByStrRef(QHash<QLatin1String, QString>&, const QStringRef&);
QStringList parseFilters(const QString& filters_raw);


QString findSystemsFile()
{
    // static const QString FALLBACK_MSG = "`%1` not found, trying next fallback";

    const QVector<QString> possible_paths = {
        homePath() % QStringLiteral("/.emulationstation/es_systems.cfg"),
        QStringLiteral("/etc/emulationstation/es_systems.cfg"),
    };

    for (const auto& path : possible_paths) {
        if (validPath(path)) {
            qInfo().noquote() << MSG_PREFIX << QObject::tr("found `%1`").arg(path);
            return path;
        }
        // qDebug() << FALLBACK_MSG.arg(path);
    }

    return QString();
}

void readSystemsFile(QXmlStreamReader& xml,
                     QHash<QString, Types::Game*>& games,
                     QHash<QString, Types::Collection*>& collections,
                     QVector<QString>& metadata_dirs)
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
    while (xml.readNextStartElement()) {
        if (xml.name() != QLatin1String("system")) {
            xml.skipCurrentElement();
            continue;
        }

        readSystemEntry(xml, games, collections, metadata_dirs);
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

    const QString& tag = xml_props[QLatin1String("name")];
    Types::Collection*& collection_ptr = collections[tag];
    if (!collection_ptr)
        collection_ptr = new Types::Collection(tag); // TODO: check for fail

    Types::Collection& collection = *collection_ptr;
    collection.setName(xml_props[QLatin1String("fullname")]);
    collection.setCommonLaunchCmd(xml_props[QLatin1String("command")]);
    collection.sourceDirsMut().append(xml_props[QLatin1String("path")]);

    // add the games

    static constexpr auto dir_filters = QDir::Files | QDir::Readable | QDir::NoDotAndDotDot;
    static constexpr auto dir_flags = QDirIterator::Subdirectories | QDirIterator::FollowSymlinks;
    const QStringList name_filters = parseFilters(xml_props[QLatin1String("extension")]);

    QDirIterator dir_it(xml_props[QLatin1String("path")], name_filters, dir_filters, dir_flags);
    while (dir_it.hasNext()) {
        const QString path = QFileInfo(dir_it.next()).canonicalFilePath();

        Types::Game*& game_ptr = games[path];
        if (!game_ptr)
            game_ptr = new Types::Game(path, &collection);

        collection.gameListMut().addGame(game_ptr);
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
    auto filter_list = filters_raw.split(" ", QString::SkipEmptyParts);
    for (auto& filter : filter_list)
        filter = filter.prepend("*").toLower();

    // remove duplicates
    return filter_list.toSet().toList();
}

} // namespace


namespace providers {

void Es2Provider::find(QHash<QString, Types::Game*>& games,
                       QHash<QString, Types::Collection*>& collections,
                       QVector<QString>& metadata_dirs)
{
    // find the systems file
    const QString xml_path = findSystemsFile();
    if (xml_path.isEmpty()) {
        qWarning().noquote() << MSG_PREFIX << QObject::tr("system config file not found");
        return;
    }

    // open the systems file
    QFile xml_file(xml_path);
    if (!xml_file.open(QIODevice::ReadOnly)) {
        qWarning().noquote() << MSG_PREFIX << QObject::tr("could not open `%1`").arg(xml_path);
        return;
    }

    // parse the systems file
    QXmlStreamReader xml(&xml_file);
    readSystemsFile(xml, games, collections, metadata_dirs);
    if (xml.error())
        qWarning().noquote() << MSG_PREFIX << xml.errorString();
}

} // namespace providers
