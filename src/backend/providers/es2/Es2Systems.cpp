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


#include "Es2Systems.h"

#include "LocaleUtils.h"
#include "Log.h"
#include "Paths.h"
#include "model/gaming/Collection.h"
#include "model/gaming/Game.h"
#include "providers/SearchContext.h"
#include "utils/CommandTokenizer.h"
#include "utils/StdHelpers.h"

#include <QFile>
#include <QFileInfo>
#include <QStringBuilder>
#include <QXmlStreamReader>
#include <array>


namespace {

QString find_systems_xml(const std::vector<QString>& possible_config_dirs)
{
    for (const QString& dir_path : possible_config_dirs) {
        QString xml_path = dir_path + QStringLiteral("es_systems.cfg");
        if (QFileInfo::exists(xml_path))
            return xml_path;
    }

    return {};
}

HashMap<QLatin1String, QString>::iterator
find_by_str_ref(HashMap<QLatin1String, QString>& map, const QStringRef& str)
{
    HashMap<QLatin1String, QString>::iterator it;
    for (it = map.begin(); it != map.end(); ++it)
        if (it->first == str)
            break;

    return it;
}


providers::es2::SystemEntry read_system_entry(const QString& log_tag, QXmlStreamReader& xml)
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
        const auto it = find_by_str_ref(xml_props, xml.name());
        if (it != xml_props.end())
            it->second = xml.readElementText();
        else
            xml.skipCurrentElement();
    }
    if (xml.error())
        return {};


    // check if all required params are present
    for (const QLatin1String& key : required_keys) {
        if (xml_props[key].isEmpty()) {
            Log::warning(log_tag, tr_log("The `<system>` node in `%1` that ends at line %2 has no `<%3>` parameter")
                .arg(static_cast<QFile*>(xml.device())->fileName(), QString::number(xml.lineNumber()), key));
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
} // namespace


namespace providers {
namespace es2 {


std::vector<SystemEntry> find_systems(const QString& log_tag, const std::vector<QString>& possible_config_dirs)
{
    const QString xml_path = find_systems_xml(possible_config_dirs);
    if (xml_path.isEmpty()) {
        Log::info(log_tag, tr_log("No installation found"));
        return {};
    }
    Log::info(log_tag, tr_log("Found `%1`").arg(xml_path));

    QFile xml_file(xml_path);
    if (!xml_file.open(QIODevice::ReadOnly)) {
        Log::error(log_tag, tr_log("Could not open `%1`").arg(xml_path));
        return {};
    }

    QXmlStreamReader xml(&xml_file);
    if (!xml.readNextStartElement()) {
        Log::error(log_tag, tr_log("Could not parse `%1`").arg(xml_path));
        return {};
    }
    if (xml.name() != QLatin1String("systemList")) {
        Log::error(log_tag, tr_log("`%1` does not have a `<systemList>` root node").arg(xml_path));
        return {};
    }

    // read all <system> nodes
    std::vector<SystemEntry> systems;
    while (xml.readNextStartElement()) {
        if (xml.name() != QLatin1String("system")) {
            xml.skipCurrentElement();
            continue;
        }

        providers::es2::SystemEntry sysentry = read_system_entry(log_tag, xml);
        if (!sysentry.name.isEmpty())
            systems.emplace_back(std::move(sysentry));
    }
    if (xml.error())
        Log::error(log_tag, xml.errorString());

    return systems;
}

} // namespace es2
} // namespace providers
