#include "Es2XmlReader.h"

#include "Model.h"
#include "Utils.h"

#include <QDir>
#include <QDebug>


QXmlStreamReader Es2XmlReader::xml;

bool Es2XmlReader::read()
{
    QString systemscfg_path = systemsCfgPath();
    QFile systemscfg(systemscfg_path);
    if (!systemscfg.open(QIODevice::ReadOnly)) {
        xml.raiseError(QObject::tr("Could not open `%1`").arg(systemscfg_path));
        return false;
    }

    xml.setDevice(&systemscfg);
    if (xml.readNextStartElement()) {
        if (xml.name() != "systemList")
            xml.raiseError(QObject::tr("`%1` does not start with a `<systemList>` node!"));
        else {
            while (xml.readNextStartElement()) {
                if (xml.name() == "system")
                    readSystem();
                else
                    xml.skipCurrentElement();
            }
        }
    }

    return !xml.error();
}

QString Es2XmlReader::systemsCfgPath()
{
    static const QString FALLBACK_MSG = "`%1` not found, trying next fallback";

    QString file_path = QDir::homePath() + "/.config/emulationstation/es_systems.cfg";
    if (validFile(file_path))
        return file_path;

    qInfo() << FALLBACK_MSG.arg(file_path);
    file_path = QDir::homePath() + "/.emulationstation/es_systems.cfg";
    if (validFile(file_path))
        return file_path;

    qInfo() << FALLBACK_MSG.arg(file_path);
    return "/etc/emulationstation/es_systems.cfg";
}

void Es2XmlReader::readSystem()
{
    Q_ASSERT(xml.isStartElement() && xml.name() == "system");

    Model::Platform platform;

    while (xml.readNextStartElement()) {
        if (xml.name() == "name")
            parseSystemShortName(platform);
        else if (xml.name() == "path")
            parseSystemRomDirPath(platform);
        else if (xml.name() == "command")
            parseSystemRunCmd(platform);
        else
            xml.skipCurrentElement();
    }

    qDebug() << platform.short_name;
    qDebug() << platform.rom_dir_path;
    qDebug() << platform.launch_cmd;
}

void Es2XmlReader::parseSystemShortName(Model::Platform& platform)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == "name");
    platform.short_name = xml.readElementText();
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
