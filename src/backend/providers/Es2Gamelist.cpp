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


#include "Es2Gamelist.h"

#include "Es2SystemsParser.h"
#include "Utils.h"

#include <QDebug>
#include <QFile>
#include <QStringBuilder>
#include <QXmlStreamReader>


namespace {

static constexpr auto MSG_PREFIX = "ES2:";

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
    es2_utils::readSystemsFile(xml, games, collections, metadata_dirs);
    if (xml.error())
        qWarning().noquote() << MSG_PREFIX << xml.errorString();
}

} // namespace providers
