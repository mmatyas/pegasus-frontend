#include "Es2XmlReader.h"

#include "Model.h"
#include "Utils.h"

#include <QDir>
#include <QDebug>


QXmlStreamReader Es2XmlReader::xml;

bool Es2XmlReader::read()
{
    QString systemscfg_path = es2SystemsCfgPath();
    QFile systemscfg(systemscfg_path);
    if (!systemscfg.open(QIODevice::ReadOnly)) {
        xml.raiseError(QObject::tr("Could not open `%1`").arg(systemscfg_path));
        return false;
    }

    xml.setDevice(&systemscfg);
    if (xml.readNextStartElement()) {
        if (xml.name() != "systemList")
            xml.raiseError(QObject::tr("`%1` must have a `systemList` node!"));
        else {
            while (xml.readNextStartElement()) {
                if (xml.name() == "system")
                    parseSystem();
                else
                    xml.skipCurrentElement();
            }
        }
    }

    return !xml.error();
}

QString Es2XmlReader::es2SystemsCfgPath()
{
    QString file_path = QDir::homePath() + "/.config/emulationstation/es_systems.cfg";
    if (validFile(file_path))
        return file_path;

    qInfo() << "`es_systems.cfg` not found under ~/.config/emulationstation, trying next fallback";
    file_path = QDir::homePath() + "/.emulationstation/es_systems.cfg";
    if (validFile(file_path))
        return file_path;

    qInfo() << "`es_systems.cfg` not found under ~/.emulationstation, trying next fallback";
    return "/etc/emulationstation/es_systems.cfg";
}

void Es2XmlReader::parseSystem()
{
    Q_ASSERT(xml.isStartElement() && xml.name() == "system");

    Model::Platform platform;

    while (xml.readNextStartElement()) {
        if (xml.name() == "name")
            parseSystemName(platform);
        else if (xml.name() == "path")
            parseSystemRomDirPath(platform);
        else if (xml.name() == "command")
            parseSystemRunCmd(platform);
        else
            xml.skipCurrentElement();
    }

    qDebug() << Platforms::shortName(platform.id);
    qDebug() << platform.rom_dir_path;
    qDebug() << platform.launch_cmd;
}

void Es2XmlReader::parseSystemName(Model::Platform& platform)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == "name");

    QString xml_text = xml.readElementText();
    if (xml_text.isEmpty())
        return;

    platform.id = Platforms::idFromShortName(xml_text);
}

void Es2XmlReader::parseSystemRomDirPath(Model::Platform& platform)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == "path");

    platform.rom_dir_path = xml.readElementText();
}

void Es2XmlReader::parseSystemRunCmd(Model::Platform& platform)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == "command");

    platform.launch_cmd = xml.readElementText();
}
