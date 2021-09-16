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

#include "Log.h"
#include "providers/launchbox/LaunchBoxXml.h"
#include "utils/PathTools.h"

#include <QDir>
#include <QFile>
#include <QXmlStreamReader>


namespace providers {
namespace launchbox {

std::vector<Platform> find_platforms(const QString& log_tag, const QDir& lb_dir)
{
    const QString xml_path = lb_dir.filePath(QStringLiteral("Data/Platforms.xml"));
    QFile xml_file(xml_path);
    if (!xml_file.open(QIODevice::ReadOnly)) {
        Log::error(log_tag, LOGMSG("Could not open `%1`").arg(::pretty_path(xml_path)));
        return {};
    }

    std::vector<Platform> out;
    QXmlStreamReader xml(&xml_file);
    verify_root_node(xml);

    while (xml.readNextStartElement() && !xml.hasError()) {
        if (xml.name() != QLatin1String("Platform")) {
            xml.skipCurrentElement();
            continue;
        }

        Platform platform;
        while (xml.readNextStartElement()) {
            if (xml.name() == QLatin1String("Name")) {
                platform.name = xml.readElementText().trimmed();
                continue;
            }
            if (xml.name() == QLatin1String("SortTitle")) {
                platform.sort_by = xml.readElementText().trimmed();
                continue;
            }
            xml.skipCurrentElement();
        }
        if (platform.name.isEmpty())
            continue;

        if (platform.sort_by.isEmpty())
            platform.sort_by = platform.name;

        out.emplace_back(std::move(platform));
    }
    if (xml.error())
        Log::error(log_tag, LOGMSG("In `%1`: %2").arg(xml_path, xml.errorString()));

    return out;
}

} // namespace launchbox
} // namespace providers
