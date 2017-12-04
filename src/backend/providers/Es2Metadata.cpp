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


#include "Es2Metadata.h"

#include "Es2GamelistParser.h"
#include "Utils.h"
#include "types/Collection.h"

#include <QDebug>
#include <QFile>
#include <QStringBuilder>
#include <QXmlStreamReader>


namespace {

static constexpr auto MSG_PREFIX = "ES2:";
static constexpr auto GAMELISTFILE = "/gamelist.xml";

QString findGamelistFile(const Types::Collection& collection)
{
    // static const QString FALLBACK_MSG = "`%1` not found, trying next fallback";

    const QVector<QString> possible_files = {
        collection.sourceDirs().constFirst() % GAMELISTFILE,
        homePath() % QStringLiteral("/.emulationstation/gamelists/") % collection.tag() % GAMELISTFILE,
        QStringLiteral("/etc/emulationstation/gamelists/") % collection.tag() % GAMELISTFILE,
    };

    for (const auto& path : possible_files) {
        if (validPath(path)) {
            qInfo().noquote() << MSG_PREFIX << QObject::tr("found `%1`").arg(path);
            return path;
        }
        // qDebug() << FALLBACK_MSG.arg(path);
    }

    return QString();
}

} // namespace


namespace providers {

void Es2Metadata::fill(const QHash<QString, Types::Game*>& games,
                       const QHash<QString, Types::Collection*>& collections,
                       const QVector<QString>&)
{
    for (const auto& collection_ptr : collections) {
        Types::Collection& collection = *collection_ptr;

        // find the metadata file
        const QString gamelist_path = findGamelistFile(collection);
        if (gamelist_path.isEmpty())
            continue;

        // open the file
        QFile xml_file(gamelist_path);
        if (!xml_file.open(QIODevice::ReadOnly)) {
            qWarning().noquote() << MSG_PREFIX
                                 << QObject::tr("could not open `%1`").arg(gamelist_path);
            continue;
        }

        // parse the file
        QXmlStreamReader xml(&xml_file);
        es2_utils::parseGamelistFile(xml, collection, games);
        if (xml.error())
            qWarning().noquote() << MSG_PREFIX << xml.errorString();
    }
}

} // namespace providers
