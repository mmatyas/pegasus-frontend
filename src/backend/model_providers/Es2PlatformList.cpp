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


#include "Es2PlatformList.h"

#include "Utils.h"
#include "types/Collection.h"

#include <QDebug>
#include <QFile>
#include <QStringBuilder>
#include <QXmlStreamReader>

static constexpr auto MSG_PREFIX = "ES2:";


namespace model_providers {

Es2PlatformList::Es2PlatformList()
    : required_system_props{"path", "command", "name", "extension"}
{
}

QVector<Types::Collection*> Es2PlatformList::find()
{
    // find the systems file
    const QString xml_path = findSystemsFile();
    if (xml_path.isEmpty()) {
        qWarning().noquote() << MSG_PREFIX << QObject::tr("system config file not found");
        return {};
    }

    // open the systems file
    QFile xml_file(xml_path);
    if (!xml_file.open(QIODevice::ReadOnly)) {
        qWarning().noquote() << MSG_PREFIX << QObject::tr("could not open `%1`").arg(xml_path);
        return {};
    }

    QXmlStreamReader xml(&xml_file);
    auto systems = parseSystemsFile(xml);
    if (xml.error()) {
        qWarning().noquote() << MSG_PREFIX << xml.errorString();
        return {};
    }
    if (systems.empty()) {
        qWarning().noquote() << MSG_PREFIX << QObject::tr("no systems found in `%1`").arg(xml_path);
    }

    return systems;
}

QString Es2PlatformList::findSystemsFile()
{
    // static const QString FALLBACK_MSG = "`%1` not found, trying next fallback";

    const QVector<QString> possible_paths = {
        homePath() % "/.emulationstation/es_systems.cfg",
        "/etc/emulationstation/es_systems.cfg",
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

QVector<Types::Collection*> Es2PlatformList::parseSystemsFile(QXmlStreamReader& xml)
{
    // read the root <systemList> element
    if (!xml.readNextStartElement()) {
        xml.raiseError(QObject::tr("could not parse `%1`")
                       .arg(static_cast<QFile*>(xml.device())->fileName()));
        return {};
    }
    if (xml.name() != "systemList") {
        xml.raiseError(QObject::tr("`%1` does not have a `<systemList>` root node!")
                       .arg(static_cast<QFile*>(xml.device())->fileName()));
        return {};
    }

    // read all <system> nodes
    QVector<Types::Collection*> collections;
    while (xml.readNextStartElement()) {
        if (xml.name() != "system") {
            xml.skipCurrentElement();
            continue;
        }

        Types::Collection* collection = parseSystemEntry(xml);
        if (collection) {
            if (!collection->shortName().isEmpty())
                collections.push_back(collection);
            else
                delete collection;
        }
    }

    return collections;
}

Types::Collection* Es2PlatformList::parseSystemEntry(QXmlStreamReader& xml)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == "system");

    // read all XML fields into a key-value map
    QHash<QString, QString> xml_props;
    while (xml.readNextStartElement())
        xml_props.insert(xml.name().toString(), xml.readElementText());

    // check if all required params are present
    for (const auto& param : required_system_props) {
        if (xml_props[param].isEmpty()) {
            qWarning().noquote()
                << MSG_PREFIX
                << QObject::tr("the `<system>` node in `%1` that ends at line #%2 has no `<%3>` parameter")
                   .arg(static_cast<QFile*>(xml.device())->fileName())
                   .arg(xml.lineNumber())
                   .arg(param);
            return nullptr;
        }
    }

    // do some path formatting
    xml_props["path"]
        .replace("\\", "/")
        .replace("~", homePath());

    // construct the new platform
    auto collection = new Types::Collection(); // TODO: check for fail
    collection->setShortName(xml_props["name"]);
    collection->searchDirsMut().append(xml_props["path"]);
    collection->romFiltersMut().append(parseFilters(xml_props["extension"]));
    collection->setCommonLaunchCmd(xml_props["command"]);
    return collection;
}

QStringList Es2PlatformList::parseFilters(const QString& str) {
    auto filter_list = str.split(" ", QString::SkipEmptyParts);
    for (auto& filter : filter_list)
        filter = filter.prepend("*").toLower();

    // remove duplicates
    return filter_list.toSet().toList();
}

} // namespace model_providers
