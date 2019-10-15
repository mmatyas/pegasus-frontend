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


#include "LaunchBoxEmulatorsXml.h"

#include "LocaleUtils.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QXmlStreamReader>


namespace providers {
namespace launchbox {

void read_xml_strings(QXmlStreamReader& xml, const HashMap<QString, QString&>& slot_map)
{
    while (xml.readNextStartElement()) {
        const auto it = slot_map.find(xml.name().toString());
        if (it == slot_map.cend()) {
            xml.skipCurrentElement();
            continue;
        }
        it->second = xml.readElementText().trimmed();
    }
}

bool read_emulator_entry(
    QXmlStreamReader& xml,
    const QString& lb_dir,
    const QLatin1String& rel_path,
    EmulatorId& out_id,
    Emulator& out_emu)
{
    const HashMap<QString, QString&> slot_map {
        { QLatin1String("ID"), out_id },
        { QLatin1String("Title"), out_emu.name },
        { QLatin1String("ApplicationPath"), out_emu.app_path },
        { QLatin1String("CommandLine"), out_emu.default_cmd_params },
    };
    read_xml_strings(xml, slot_map);

    if (out_id.isEmpty()) {
        log_xml_warning(xml, rel_path, tr_log("empty emulator ID detected, entry ignored"));
        return false;
    }
    if (out_emu.name.isEmpty()) {
        log_xml_warning(xml, rel_path, tr_log("no emulator name found, entry ignored"));
        return false;
    }

    const QFileInfo emu_finfo(lb_dir, out_emu.app_path);
    QString can_path = emu_finfo.canonicalFilePath();
    if (!can_path.isEmpty()) {
        out_emu.app_path = std::move(can_path);
    } else {
        log_xml_warning(xml, rel_path,
            tr_log("emulator application `%1` doesn't seem to exist, entry ignored")
                .arg(QDir::toNativeSeparators(emu_finfo.absoluteFilePath())));
        return false;
    }

    return true;
}

bool read_emu_platform_entry(
    QXmlStreamReader& xml,
    const QLatin1String& rel_path,
    EmulatorId& out_id,
    EmulatorPlatform& out_platform)
{
    const HashMap<QString, QString&> slot_map {
        { QLatin1String("Emulator"), out_id },
        { QLatin1String("Platform"), out_platform.name },
        { QLatin1String("CommandLine"), out_platform.cmd_params },
    };
    read_xml_strings(xml, slot_map);

    if (out_id.isEmpty()) {
        log_xml_warning(xml, rel_path, tr_log("empty emulator ID detected, entry ignored"));
        return false;
    }
    if (out_platform.name.isEmpty()) {
        log_xml_warning(xml, rel_path, tr_log("no platform name found, entry ignored"));
        return false;
    }

    return true;
}
} // namespace launchbox
} // namespace providers


namespace providers {
namespace launchbox {
namespace emulators_xml {

HashMap<EmulatorId, Emulator> read(const QString& lb_dir)
{
    const QLatin1String xml_rel_path("Data/Emulators.xml");
    const QString xml_path = lb_dir + xml_rel_path;
    QFile xml_file(xml_path);
    if (!xml_file.open(QIODevice::ReadOnly)) {
        qWarning().noquote() << MSG_PREFIX << tr_log("could not open `%1`")
            .arg(QDir::toNativeSeparators(xml_rel_path));
        return {};
    }


    HashMap<EmulatorId, Emulator> emus_by_id;
    HashMap<EmulatorId, std::vector<EmulatorPlatform>> emu_platforms_for_id;

    QXmlStreamReader xml(&xml_file);
    check_lb_root_node(xml, xml_rel_path);

    while (xml.readNextStartElement() && !xml.hasError()) {
        if (xml.name() == QLatin1String("Emulator")) {
            EmulatorId emu_id;
            Emulator emu;
            if (read_emulator_entry(xml, lb_dir, xml_rel_path, emu_id, emu))
                emus_by_id.emplace(std::move(emu_id), std::move(emu)); // assume no collision
            continue;
        }
        if (xml.name() == QLatin1String("EmulatorPlatform")) {
            EmulatorId emu_id;
            EmulatorPlatform platform;
            if (read_emu_platform_entry(xml, xml_rel_path, emu_id, platform))
                emu_platforms_for_id[std::move(emu_id)].emplace_back(std::move(platform)); // assume no collision
            continue;
        }
        xml.skipCurrentElement();
    }
    if (xml.error())
        qWarning().noquote() << MSG_PREFIX << xml.errorString();


    for (const auto& entry : emu_platforms_for_id) {
        const auto emu_it = emus_by_id.find(entry.first);
        if (emu_it == emus_by_id.cend()) {
            for (const EmulatorPlatform& platform : entry.second) {
                qWarning().noquote() << MSG_PREFIX
                    << tr_log("%1: emulator platform `%2` refers to a missing or invalid emulator entry with id `%3`, entry ignored")
                        .arg(QDir::toNativeSeparators(xml_rel_path), platform.name, entry.first);
            }
            continue;
        }
        emu_it->second.platforms = std::move(entry.second);
    }

    return emus_by_id;
}

} // namespace emulators_xml
} // namespace launchbox
} // namespace providers
