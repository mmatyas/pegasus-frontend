#include "Es2XmlReader.h"

#include "Model.h"
#include "Utils.h"

#include <QDir>
#include <QDebug>


QXmlStreamReader Es2XmlReader::xml;

bool Es2XmlReader::read(Model::PlatformModel& platform_model)
{
    QVector<Model::PlatformItemPtr> platforms = readSystemsFile();
    if (xml.error()) {
        qWarning() << xml.errorString();
        return false;
    }

    for (auto& platform : platforms) {
        QVector<Model::GameItemPtr> games = readGamelistFile(platform);
        if (xml.error())
            qWarning() << xml.errorString();

        for (auto& game : games)
            platform->game_model->append(game);
        platform_model.append(platform);
    }

    return true;
}

QVector<Model::PlatformItemPtr> Es2XmlReader::readSystemsFile()
{
    QString systemscfg_path = systemsCfgPath();
    QFile systemscfg(systemscfg_path);
    if (!systemscfg.open(QIODevice::ReadOnly)) {
        xml.raiseError(QObject::tr("Could not open `%1`").arg(systemscfg_path));
        return {};
    }

    QVector<Model::PlatformItemPtr> platforms;

    xml.setDevice(&systemscfg);
    if (xml.readNextStartElement()) {
        if (xml.name() != "systemList")
            xml.raiseError(QObject::tr("`%1` does not start with a `<systemList>` node!"));
        else {
            while (xml.readNextStartElement()) {
                if (xml.name() == "system") {
                    Model::PlatformItemPtr platform = readSystem();
                    if (!platform->short_name.isEmpty())
                        platforms.push_back(platform);
                }
                else
                    xml.skipCurrentElement();
            }
        }
    }

    return platforms;
}

QString Es2XmlReader::systemsCfgPath()
{
    static const QString FALLBACK_MSG = "`%1` not found, trying next fallback";

    QString file_path = QDir::homePath() + "/.config/emulationstation/es_systems.cfg";
    if (validFile(file_path))
        return file_path;

    // qInfo() << FALLBACK_MSG.arg(file_path);
    file_path = QDir::homePath() + "/.emulationstation/es_systems.cfg";
    if (validFile(file_path))
        return file_path;

    // qInfo() << FALLBACK_MSG.arg(file_path);
    return "/etc/emulationstation/es_systems.cfg";
}

Model::PlatformItemPtr Es2XmlReader::readSystem()
{
    Q_ASSERT(xml.isStartElement() && xml.name() == "system");

    Model::PlatformItemPtr platform(new Model::PlatformItem);

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

    return platform;
}

void Es2XmlReader::parseSystemShortName(Model::PlatformItemPtr& platform) {
    Q_ASSERT(xml.isStartElement() && xml.name() == "name");
    platform->short_name = xml.readElementText();
}

void Es2XmlReader::parseSystemRomDirPath(Model::PlatformItemPtr& platform) {
    Q_ASSERT(xml.isStartElement() && xml.name() == "path");
    platform->rom_dir_path = xml.readElementText();
}

void Es2XmlReader::parseSystemRunCmd(Model::PlatformItemPtr& platform) {
    Q_ASSERT(xml.isStartElement() && xml.name() == "command");
    platform->launch_cmd = xml.readElementText();
}

QVector<Model::GameItemPtr> Es2XmlReader::readGamelistFile(const Model::PlatformItemPtr& platform)
{
    QString gamelist_path = gamelistPath(platform->short_name);
    QFile gamelist(gamelist_path);
    if (!gamelist.open(QIODevice::ReadOnly)) {
        xml.raiseError(QObject::tr("Could not open `%1`").arg(gamelist_path));
        return {};
    }

    QVector<Model::GameItemPtr> games;

    xml.setDevice(&gamelist);
    if (xml.readNextStartElement()) {
        if (xml.name() != "gameList")
            xml.raiseError(QObject::tr("`%1` does not start with a `<gameList>` node!"));
        else {
            while (xml.readNextStartElement()) {
                if (xml.name() == "game") {
                    Model::GameItemPtr game = readGame();
                    if (!game->rom_path.isEmpty())
                        games.push_back(game);
                }
                else
                    xml.skipCurrentElement();
            }
        }
    }

    return games;
}

QString Es2XmlReader::gamelistPath(const QString& platform_name)
{
    static const QString FALLBACK_MSG = "`%1` not found, trying next fallback";

    if (platform_name.isEmpty())
        return "";

    const QString path_suffix = "gamelists/" + platform_name + "/gamelist.xml";
    QString file_path = QDir::homePath() + "/.config/emulationstation/" + path_suffix;
    if (validFile(file_path))
        return file_path;

    // qInfo() << FALLBACK_MSG.arg(file_path);
    file_path = QDir::homePath() + "/.emulationstation/" + path_suffix;
    if (validFile(file_path))
        return file_path;

    // qInfo() << FALLBACK_MSG.arg(file_path);
    return "/etc/emulationstation/" + path_suffix;
}

Model::GameItemPtr Es2XmlReader::readGame()
{
    Q_ASSERT(xml.isStartElement() && xml.name() == "game");

    Model::GameItemPtr game(new Model::GameItem);

    while (xml.readNextStartElement()) {
        if (xml.name() == "path")
            parseGamePath(game);
        else if (xml.name() == "name")
            parseGameName(game);
        else if (xml.name() == "desc")
            parseGameDescription(game);
        else if (xml.name() == "developer")
            parseGameDeveloper(game);
        else if (xml.name() == "image")
            parseGameImage(game);
        else
            xml.skipCurrentElement();
    }

    return game;
}

void Es2XmlReader::parseGamePath(Model::GameItemPtr& game) {
    Q_ASSERT(xml.isStartElement() && xml.name() == "path");
    game->rom_path = xml.readElementText();
}

void Es2XmlReader::parseGameName(Model::GameItemPtr& game) {
    Q_ASSERT(xml.isStartElement() && xml.name() == "name");
    game->title = xml.readElementText();
}

void Es2XmlReader::parseGameDescription(Model::GameItemPtr& game) {
    Q_ASSERT(xml.isStartElement() && xml.name() == "desc");
    game->description = xml.readElementText();
}

void Es2XmlReader::parseGameDeveloper(Model::GameItemPtr& game) {
    Q_ASSERT(xml.isStartElement() && xml.name() == "developer");
    game->developer = xml.readElementText();
}

void Es2XmlReader::parseGameImage(Model::GameItemPtr& /*game*/) {
    Q_ASSERT(xml.isStartElement() && xml.name() == "image");
    /*game->assets.box_front_path =*/ xml.readElementText();
}
