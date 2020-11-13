// Pegasus Frontend
// Copyright (C) 2017-2020  Mátyás Mustoha
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


#include "LaunchBoxPlatformsXml.h"

#include "LocaleUtils.h"
#include "Log.h"
#include "providers/launchbox/LaunchBoxXml.h"

#include <QDir>
#include <QFile>
#include <QXmlStreamReader>


namespace providers {
namespace launchbox {

std::vector<QString> find_platforms(const QString& log_tag, const QDir& lb_dir)
{
    const QString xml_path = lb_dir.filePath(QStringLiteral("Data/Platforms.xml"));
    QFile xml_file(xml_path);
    if (!xml_file.open(QIODevice::ReadOnly)) {
        Log::error(tr_log("%1: Could not open `%2`").arg(log_tag, QDir::toNativeSeparators(xml_path)));
        return {};
    }

    std::vector<QString> out;

    QXmlStreamReader xml(&xml_file);
    verify_root_node(xml);

    while (xml.readNextStartElement() && !xml.hasError()) {
        if (xml.name() != QLatin1String("Platform")) {
            xml.skipCurrentElement();
            continue;
        }
        while (xml.readNextStartElement()) {
            if (xml.name() != QLatin1String("Name")) {
                xml.skipCurrentElement();
                continue;
            }

            QString text = xml.readElementText().trimmed();
            if (!text.isEmpty())
                out.emplace_back(std::move(text));
        }
    }
    if (xml.error())
        Log::error(tr_log("%1: `%2`: %3").arg(log_tag, xml_path, xml.errorString()));

    return out;
}

} // namespace launchbox
} // namespace providers
