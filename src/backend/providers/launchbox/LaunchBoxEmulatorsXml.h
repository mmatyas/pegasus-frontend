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


#pragma once

#include "utils/HashMap.h"

#include <QString>
#include <QDir>

class QXmlStreamReader;


namespace providers {
namespace launchbox {

enum class EmulatorField : unsigned char;
enum class PlatformField : unsigned char;
struct Emulator;

class EmulatorsXml {
public:
    explicit EmulatorsXml(QString, QDir);

    HashMap<QString, Emulator> find() const;

private:
    const QString m_log_tag;
    const QDir m_lb_root;

    const HashMap<QString, EmulatorField> m_emulator_keys;
    const HashMap<QString, PlatformField> m_platform_keys;

    void log_xml_warning(const QString&, const size_t, const QString&) const;
    HashMap<EmulatorField, QString> read_emulator_node(QXmlStreamReader&) const;
    HashMap<PlatformField, QString> read_platform_node(QXmlStreamReader&) const;
    bool emulator_fields_valid(const QString&, const size_t, const HashMap<EmulatorField, QString>&) const;
    bool platform_fields_valid(const QString&, const size_t, const HashMap<PlatformField, QString>&) const;
};

HashMap<QString, Emulator> find_emulators(const QString&, const QDir&);

} // namespace launchbox
} // namespace providers
