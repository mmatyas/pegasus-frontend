#include "Es2Systems.h"

#include "Es2Gamelist.h"
#include "Model.h"
#include "Utils.h"

#include <QDebug>
#include <QDir>
#include <QDirIterator>


namespace Es2 {

QXmlStreamReader Systems::xml;

QVector<Model::Platform*> Systems::read()
{
    // reset
    xml.clear();

    // find the file
    const QString xml_path = findSystemsFile();
    if (xml_path.isEmpty()) {
        qWarning().noquote() << QObject::tr("ES2 system config not found");
        return {};
    }

    // open the file
    QFile xml_file(xml_path);
    if (!xml_file.open(QIODevice::ReadOnly)) {
        qWarning().noquote() << QObject::tr("Could not open `%1`").arg(xml_path);
        return {};
    }

    auto systems = parseSystemsFile(xml_file);
    if (xml.error()) {
        qWarning().noquote() << xml.errorString();
        return {};
    }

    return systems;
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

QVector<Model::Platform*> Systems::parseSystemsFile(QFile& xml_file)
{
    xml.setDevice(&xml_file);

    // read the root <systemList> element
    if (!xml.readNextStartElement()) {
        xml.raiseError(QObject::tr("Could not parse `%1`").arg(xml_file.fileName()));
        return {};
    }
    if (xml.name() != "systemList") {
        xml.raiseError(QObject::tr("`%1` does not have a `<systemList>` root node!")
                       .arg(xml_file.fileName()));
        return {};
    }

    // read all <system> nodes
    QVector<Model::Platform*> platforms;
    while (xml.readNextStartElement()) {
        if (xml.name() != "system") {
            xml.skipCurrentElement();
            continue;
        }

        Model::Platform* platform = parseSystemTag();
        if (!platform->m_short_name.isEmpty())
            platforms.push_back(platform);
    }

    return platforms;
}

Model::Platform* Systems::parseSystemTag()
{
    Q_ASSERT(xml.isStartElement() && xml.name() == "system");

    // read all XML fields into a key-value map
    QHash<QString, QString> xml_props;
    while (xml.readNextStartElement())
        xml_props.insert(xml.name().toString(), xml.readElementText());

    // check if all required params are present
    const auto required_params = {"path", "command", "name", "extension"};
    for (const auto& param : required_params) {
        if (xml_props[param].isEmpty()) {
            qWarning().noquote()
                << QObject::tr("Required parameter <%1> is missing or empty in a <system> node")
                   .arg(param);
            return nullptr;
        }
    }

    // do some post processing
    processRomDir(xml_props["path"]);

    // construct the new platform
    return new Model::Platform(
        xml_props["name"],
        xml_props["path"],
        parseFilters(xml_props["extension"]),
        xml_props["command"]);
}

void Systems::processRomDir(QString& path) {
    path.replace("\\", "/")
        .replace("~", QDir::homePath());
}

QStringList Systems::parseFilters(const QString& str) {
    auto filter_list = str.split(" ", QString::SkipEmptyParts);
    for (auto& filter : filter_list)
        filter = filter.prepend("*").toLower();

    // remove duplicates
    return filter_list.toSet().toList();
}

} // namespace Es2
