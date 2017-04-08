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
        if (xml.error()) {
            // qWarning() << xml.errorString();
            continue;
        }

        for (Model::GameItemPtr& game : games) {
            findGameAssets(platform, game);
            platform->game_model.append(game);
        }

        if (platform->game_model.rowCount() > 0)
            platform_model.append(platform);
    }

    return true;
}

QVector<Model::PlatformItemPtr> Es2XmlReader::readSystemsFile()
{
    QString systemscfg_path = systemsCfgPath();
    if (systemscfg_path.isEmpty()) {
        xml.raiseError(QObject::tr("System config not found"));
        return {};
    }

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
    static const QString FOUND_MSG = "Found `%1`";
    static const QString FALLBACK_MSG = "`%1` not found, trying next fallback";

    static const QVector<QString> possible_paths = {
        QDir::homePath() + "/.config/emulationstation/es_systems.cfg",
        QDir::homePath() + "/.emulationstation/es_systems.cfg",
        "/etc/emulationstation/es_systems.cfg",
    };

    for (const auto& path : possible_paths) {
        if (validFile(path)) {
            qInfo() << FOUND_MSG.arg(path);
            return path;
        }
        // qDebug() << FALLBACK_MSG.arg(path);
    }

    return QString();
}

Model::PlatformItemPtr Es2XmlReader::readSystem()
{
    Q_ASSERT(xml.isStartElement() && xml.name() == "system");

    Model::PlatformItemPtr platform(new Model::PlatformItem());

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
    QString gamelist_path = gamelistPath(platform);
    if (gamelist_path.isEmpty()) {
        xml.raiseError(QObject::tr("Gamelist for platform `%1` not found").arg(platform->short_name));
        return {};
    }

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

QString Es2XmlReader::gamelistPath(const Model::PlatformItemPtr& platform)
{
    static constexpr auto FILENAME = "/gamelist.xml";
    static const QString FOUND_MSG = "Found `%1`";
    static const QString FALLBACK_MSG = "`%1` not found, trying next fallback";

    if (platform->short_name.isEmpty())
        return "";

    const QString path_suffix = "/gamelists/" + platform->short_name + FILENAME;
    const QVector<QString> possible_paths = {
        platform->rom_dir_path + FILENAME,
        QDir::homePath() + "/.config/emulationstation" + path_suffix,
        QDir::homePath() + "/.emulationstation" + path_suffix,
        "/etc/emulationstation" + path_suffix,
    };

    for (const auto& path : possible_paths) {
        if (validFile(path)) {
            qInfo() << FOUND_MSG.arg(path);
            return path;
        }
        // qDebug() << FALLBACK_MSG.arg(path);
    }

    return QString();
}

Model::GameItemPtr Es2XmlReader::readGame()
{
    Q_ASSERT(xml.isStartElement() && xml.name() == "game");

    Model::GameItemPtr game(new Model::GameItem());

    while (xml.readNextStartElement()) {
        if (xml.name() == "path")
            parseGamePath(game);
        else if (xml.name() == "name")
            parseGameName(game);
        else if (xml.name() == "desc")
            parseGameDescription(game);
        else if (xml.name() == "developer")
            parseGameDeveloper(game);
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

QString Es2XmlReader::gameAssetPath(const Model::PlatformItemPtr& platform,
                                    const Model::GameItemPtr& game,
                                    const QString& asset_suffix)
{
    static const QString FALLBACK_MSG = "`%1` not found, trying next fallback";

    Q_ASSERT(!asset_suffix.isEmpty());
    if (platform->short_name.isEmpty() || game->rom_path.isEmpty())
        return "";

    // first, find out the base name of the rom
    QFileInfo rom_path_info(game->rom_path);
    const QString rom_basename = rom_path_info.completeBaseName();
    if (rom_basename.isEmpty())
        return "";

    // then search for the asset in all supported paths
    const QString path_suffix = "downloaded_images/" + platform->short_name
                              + "/" + rom_basename + asset_suffix;
    QString file_path = QDir::homePath() + "/.config/emulationstation/" + path_suffix;
    if (validFile(file_path))
        return file_path;

    // qInfo() << FALLBACK_MSG.arg(file_path);
    file_path = QDir::homePath() + "/.emulationstation/" + path_suffix;
    if (validFile(file_path))
        return file_path;

    // qInfo() << FALLBACK_MSG.arg(file_path);
    file_path = "/etc/emulationstation/" + path_suffix;
    if (validFile(file_path))
        return file_path;

    return "";
}

void Es2XmlReader::findGameAssets(Model::PlatformItemPtr& platform, Model::GameItemPtr& game)
{
    Model::GameAssets& assets = game->assets;

    assets.box_front = gameAssetPath(platform, game, "-boxart2D.png");
    if (assets.box_front.isEmpty())
        assets.box_front = gameAssetPath(platform, game, "-boxart2D.jpg");
    if (assets.box_front.isEmpty())
        assets.box_front = gameAssetPath(platform, game, ".png");
    if (assets.box_front.isEmpty())
        assets.box_front = gameAssetPath(platform, game, ".jpg");

    assets.logo = gameAssetPath(platform, game, "-logo.png");
    if (assets.logo.isEmpty())
        assets.logo = gameAssetPath(platform, game, "-logo.jpg");

    // TODO: support multiple
    QString screenshot_path = gameAssetPath(platform, game, "-screenshot.png");
    if (screenshot_path.isEmpty())
        screenshot_path = gameAssetPath(platform, game, "-screenshot.jpg");

    assets.screenshot_list << screenshot_path;

    // TODO: support multiple
    QString video_path = gameAssetPath(platform, game, "-video.webm");
    if (video_path.isEmpty())
        video_path = gameAssetPath(platform, game, "-screenshot.mp4");
    if (video_path.isEmpty())
        video_path = gameAssetPath(platform, game, "-screenshot.avi");

    assets.video_list << video_path;
}
