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


#include "LaunchBoxEmulatorsXml.h"

#include "LocaleUtils.h"
#include "Log.h"
#include "providers/launchbox/LaunchBoxEmulator.h"
#include "providers/launchbox/LaunchBoxXml.h"

#include <QDir>
#include <QFileInfo>
#include <QXmlStreamReader>


namespace providers {
namespace launchbox {

enum class EmulatorField : unsigned char {
    ID,
    NAME,
    PATH,
    PARAMS,
};

enum class PlatformField : unsigned char {
    EMULATOR,
    NAME,
    PARAMS,
};


void apply_emulator_fields(const HashMap<EmulatorField, QString>& fields, Emulator& emu)
{
    // TODO: C++17
    for (const auto& pair : fields) {
        switch (pair.first) {
            case EmulatorField::NAME:
                emu.name = pair.second;
                break;
            case EmulatorField::PATH:
                emu.app_path = pair.second;
                break;
            case EmulatorField::PARAMS:
                emu.default_cmd_params = pair.second;
                break;
            case EmulatorField::ID:
                break;
        }
    }
}

void apply_platform_fields(const HashMap<PlatformField, QString>& fields, EmulatorPlatform& emu)
{
    // TODO: C++17
    for (const auto& pair : fields) {
        switch (pair.first) {
            case PlatformField::NAME:
                emu.name = pair.second;
                break;
            case PlatformField::PARAMS:
                emu.cmd_params = pair.second;
                break;
            case PlatformField::EMULATOR:
                break;
        }
    }
}

} // namespace launchbox
} // namespace providers


namespace providers {
namespace launchbox {

EmulatorsXml::EmulatorsXml(QString log_tag, QDir lb_root)
    : m_log_tag(std::move(log_tag))
    , m_lb_root(std::move(lb_root))
    , m_emulator_keys {
        { QStringLiteral("ID"), EmulatorField::ID },
        { QStringLiteral("Title"), EmulatorField::NAME },
        { QStringLiteral("ApplicationPath"), EmulatorField::PATH },
        { QStringLiteral("CommandLine"), EmulatorField::PARAMS },
    }
    , m_platform_keys {
        { QStringLiteral("Emulator"), PlatformField::EMULATOR },
        { QStringLiteral("Platform"), PlatformField::NAME },
        { QStringLiteral("CommandLine"), PlatformField::PARAMS },
    }
{}

void EmulatorsXml::log_xml_warning(const QString& xml_path, const size_t linenum, const QString& msg) const
{
    Log::warning(m_log_tag, tr_log("In `%1` at line %2: %3")
        .arg(QDir::toNativeSeparators(xml_path), QString::number(linenum), msg));
}

bool EmulatorsXml::platform_fields_valid(
    const QString& xml_path,
    const size_t xml_linenum,
    const HashMap<PlatformField, QString>& fields) const
{
    const auto it_id = fields.find(PlatformField::EMULATOR);
    if (it_id == fields.cend()) {
        log_xml_warning(xml_path, xml_linenum, tr_log("Emulator platform has no emulator ID field, entry ignored"));
        return false;
    }

    const auto it_name = fields.find(PlatformField::NAME);
    if (it_name == fields.cend()) {
        log_xml_warning(xml_path, xml_linenum, tr_log("Emulator platform has no name, entry ignored"));
        return false;
    }

    return true;
}

bool EmulatorsXml::emulator_fields_valid(
    const QString& xml_path,
    const size_t xml_linenum,
    const HashMap<EmulatorField, QString>& fields) const
{
    const auto it_id = fields.find(EmulatorField::ID);
    if (it_id == fields.cend()) {
        log_xml_warning(xml_path, xml_linenum, tr_log("Emulator has no ID field, entry ignored"));
        return false;
    }

    const auto it_name = fields.find(EmulatorField::NAME);
    if (it_name == fields.cend()) {
        log_xml_warning(xml_path, xml_linenum, tr_log("Emulator has no name, entry ignored"));
        return false;
    }

    const auto it_path = fields.find(EmulatorField::PATH);
    if (it_path == fields.cend()) {
        log_xml_warning(xml_path, xml_linenum, tr_log("Emulator has no launchable executable, entry ignored"));
        return false;
    }

    if (!QFileInfo::exists(it_path->second)) {
        const QString display_path = QDir::toNativeSeparators(it_path->second);
        log_xml_warning(xml_path, xml_linenum,
            tr_log("Emulator executable `%1` doesn't seem to exist, entry ignored").arg(display_path));
        return false;
    }

    return true;
}

HashMap<EmulatorField, QString> EmulatorsXml::read_emulator_node(QXmlStreamReader& xml) const
{
    HashMap<EmulatorField, QString> fields;

    while (xml.readNextStartElement()) {
        const auto it = m_emulator_keys.find(xml.name().toString());
        if (it == m_emulator_keys.cend()) {
            xml.skipCurrentElement();
            continue;
        }

        QString contents = xml.readElementText().trimmed();
        if (!contents.isEmpty())
            fields.emplace(it->second, std::move(contents));
    }


    const auto it = fields.find(EmulatorField::PATH);
    if (it != fields.cend())
        it->second = QFileInfo(m_lb_root, it->second).absoluteFilePath();


    return fields;
}

HashMap<PlatformField, QString> EmulatorsXml::read_platform_node(QXmlStreamReader& xml) const
{
    HashMap<PlatformField, QString> fields;

    while (xml.readNextStartElement()) {
        const auto it = m_platform_keys.find(xml.name().toString());
        if (it == m_platform_keys.cend()) {
            xml.skipCurrentElement();
            continue;
        }

        QString contents = xml.readElementText().trimmed();
        if (!contents.isEmpty())
            fields.emplace(it->second, std::move(contents));
    }

    return fields;
}

HashMap<QString, Emulator> EmulatorsXml::find() const
{
    const QString xml_path = m_lb_root.filePath(QStringLiteral("Data/Emulators.xml"));
    QFile xml_file(xml_path);
    if (!xml_file.open(QIODevice::ReadOnly)) {
        Log::error(m_log_tag, tr_log("Could not open `%1`").arg(QDir::toNativeSeparators(xml_path)));
        return {};
    }


    HashMap<QString, Emulator> emulators;
    HashMap<QString, std::vector<EmulatorPlatform>> emulator_platforms;

    QXmlStreamReader xml(&xml_file);
    verify_root_node(xml);

    while (xml.readNextStartElement()) {
        if (xml.name() == QLatin1String("Emulator")) {
            const size_t linenum = xml.lineNumber();

            const HashMap<EmulatorField, QString> fields = read_emulator_node(xml);
            const bool node_valid = emulator_fields_valid(xml_path, linenum, fields);
            if (!node_valid)
                continue;

            QString emu_id = fields.at(EmulatorField::ID);
            Emulator emu;
            apply_emulator_fields(fields, emu);
            emulators.emplace(std::move(emu_id), std::move(emu));
            continue;
        }

        if (xml.name() == QLatin1String("EmulatorPlatform")) {
            const size_t linenum = xml.lineNumber();

            const HashMap<PlatformField, QString> fields = read_platform_node(xml);
            const bool node_valid = platform_fields_valid(xml_path, linenum, fields);
            if (!node_valid)
                continue;

            QString emu_id = fields.at(PlatformField::EMULATOR);
            EmulatorPlatform platform;
            apply_platform_fields(fields, platform);
            emulator_platforms[std::move(emu_id)].emplace_back(std::move(platform));
            continue;
        }

        xml.skipCurrentElement();
    }
    if (xml.error())
        Log::error(m_log_tag, tr_log("`%1`: %2").arg(xml_path, xml.errorString()));


    // TODO: C++17
    for (const auto& entry : emulator_platforms) {
        const auto it = emulators.find(entry.first);
        if (it == emulators.cend()) {
            for (const EmulatorPlatform& platform : entry.second) {
                Log::warning(m_log_tag, tr_log("In `%1` emulator platform `%2` refers to a missing or invalid emulator entry with id `%3`, entry ignored")
                    .arg(QDir::toNativeSeparators(xml_path), platform.name, entry.first));
            }
            continue;
        }

        it->second.platforms = std::move(entry.second);
    }


    return emulators;
}

} // namespace launchbox
} // namespace providers
