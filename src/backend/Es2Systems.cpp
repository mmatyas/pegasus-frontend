#include "Es2Systems.h"

#include "Es2Gamelist.h"
#include "Model.h"
#include "Utils.h"

#include <QDebug>
#include <QDir>
#include <QDirIterator>


namespace Es2 {

QString Systems::xml_path;
QXmlStreamReader Systems::xml;

QVector<Model::Platform*> Systems::read()
{
    QVector<Model::Platform*> platforms = readSystemsFile();
    if (xml.error()) {
        qWarning().noquote() << xml.errorString();
        return {};
    }
    return platforms;
}

QVector<Model::Platform*> Systems::readSystemsFile()
{
    xml.clear();
    xml_path.clear();

    xml_path = findSystemsFile();
    if (xml_path.isEmpty()) {
        xml.raiseError(QObject::tr("ES2 system config not found"));
        return {};
    }

    QFile xml_file(xml_path);
    openSystemsFile(xml_file);
    if (!xml.device())
        return {};

    return parseSystemsFile();
}

QString Systems::findSystemsFile()
{
    // static const QString FALLBACK_MSG = "`%1` not found, trying next fallback";

    const QVector<QString> possible_paths = {
        QDir::homePath() + "/.config/emulationstation/es_systems.cfg",
        QDir::homePath() + "/.emulationstation/es_systems.cfg",
        "/etc/emulationstation/es_systems.cfg",
    };

    for (const auto& path : possible_paths) {
        if (validFile(path)) {
            qInfo().noquote() << QStringLiteral("Found `%1`").arg(path);
            return path;
        }
        // qDebug() << FALLBACK_MSG.arg(path);
    }

    return QString();
}

void Systems::openSystemsFile(QFile& xml_file)
{
    Q_ASSERT(!xml.device());
    Q_ASSERT(!xml_path.isEmpty());

    if (!xml_file.open(QIODevice::ReadOnly)) {
        xml.raiseError(QObject::tr("Could not open `%1`").arg(xml_path));
        return;
    }

    xml.setDevice(&xml_file);
}

QVector<Model::Platform*> Systems::parseSystemsFile()
{
    Q_ASSERT(xml.device() && !xml.atEnd());

    QVector<Model::Platform*> platforms;

    // read the root <systemList> element
    if (xml.readNextStartElement()) {
        if (xml.name() != "systemList") {
            xml.raiseError(QObject::tr("`%1` does not have a `<systemList>` root node!")
                           .arg(xml_path));
        }
        else {
            // read all <system> nodes
            while (xml.readNextStartElement()) {
                if (xml.name() != "system") {
                    xml.skipCurrentElement();
                    continue;
                }

                Model::Platform* platform = readSystemTag();
                if (!platform->m_short_name.isEmpty())
                    platforms.push_back(platform);
            }
        }
    }
    else {
        xml.raiseError(QObject::tr("Could not parse `%1`").arg(xml_path));
    }

    return platforms;
}

Model::Platform* Systems::readSystemTag()
{
    Q_ASSERT(xml.isStartElement() && xml.name() == "system");

    Model::Platform* platform(new Model::Platform());

    while (xml.readNextStartElement()) {
        if (xml.name() == "name")
            parseSystemShortName(platform);
        else if (xml.name() == "path")
            parseSystemRomDirPath(platform);
        else if (xml.name() == "command")
            parseSystemRunCmd(platform);
        else if (xml.name() == "extension")
            parseSystemExtensions(platform);
        else
            xml.skipCurrentElement();
    }

    return platform;
}

void Systems::parseSystemShortName(Model::Platform* platform) {
    Q_ASSERT(xml.isStartElement() && xml.name() == "name");
    platform->m_short_name = xml.readElementText();
}

void Systems::parseSystemRomDirPath(Model::Platform* platform) {
    Q_ASSERT(xml.isStartElement() && xml.name() == "path");
    platform->m_rom_dir_path = xml.readElementText()
        .replace("\\", "/")
        .replace("~", QDir::homePath());
}

void Systems::parseSystemRunCmd(Model::Platform* platform) {
    Q_ASSERT(xml.isStartElement() && xml.name() == "command");
    platform->m_launch_cmd = xml.readElementText();
}

void Systems::parseSystemExtensions(Model::Platform* platform) {
    Q_ASSERT(xml.isStartElement() && xml.name() == "extension");

    platform->m_rom_filters = xml.readElementText().split(" ", QString::SkipEmptyParts);
    for (auto& filter : platform->m_rom_filters)
        filter = filter.prepend("*").toLower();

    // remove duplicates
    platform->m_rom_filters = platform->m_rom_filters.toSet().toList();
}

} // namespace Es2
