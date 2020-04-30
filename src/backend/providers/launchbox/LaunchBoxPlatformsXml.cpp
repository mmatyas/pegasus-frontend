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


#include "LaunchBoxPlatformsXml.h"

#include "LaunchBoxCommon.h"
#include "LocaleUtils.h"
#include "providers/Provider.h"

#include <QDebug>
#include <QDir>
#include <QFile>


namespace providers {
namespace launchbox {
namespace platforms_xml {

std::vector<QString> read(const providers::Provider* const provider, const QString& lb_dir)
{
    const QLatin1String xml_rel_path("Data/Platforms.xml");
    const QString xml_path = lb_dir + xml_rel_path;
    QFile xml_file(xml_path);
    if (!xml_file.open(QIODevice::ReadOnly)) {
        provider->warn(tr_log("could not open `%1`")
            .arg(QDir::toNativeSeparators(xml_rel_path)));
        return {};
    }

    std::vector<QString> out;

    QXmlStreamReader xml(&xml_file);
    check_lb_root_node(xml, xml_rel_path);

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
        provider->warn(xml.errorString());

    return out;
}

} // namespace platforms_xml
} // namespace launchbox
} // namespace providers
